from typing import List, Optional, Tuple

import scalellm._C.kernels as kernels  # type: ignore

import pytest
import torch

from ref_attention import varlen_masked_self_attention


@pytest.mark.parametrize("seq_lens", [[(1, 100), (5, 15), (111, 234)]])
@pytest.mark.parametrize("num_heads", [(8, 8), (8, 4), (8, 2), (8, 1)])
@pytest.mark.parametrize("head_size", [64, 128, 256])
@pytest.mark.parametrize("n_blocks", [100])
@pytest.mark.parametrize("block_size", [4, 8, 16, 32])
@pytest.mark.parametrize("dtype", [torch.float16, torch.bfloat16])
@pytest.mark.parametrize("logits_soft_cap", [0.0, 30.0, 50.0])
# @pytest.mark.parametrize("sliding_window", [-1, 128])
# @pytest.mark.parametrize("alibi", [False, True])
@torch.inference_mode
def test_flashinfer_varlen_masked_self_attention(
    seq_lens: List[Tuple[int, int]],
    num_heads: Tuple[int, int],
    head_size: int,
    dtype: torch.dtype,
    n_blocks: int,
    block_size: int,
    logits_soft_cap: Optional[float],
) -> None:
    torch.set_default_device("cuda")

    n_seqs = len(seq_lens)
    q_lens = [x[0] for x in seq_lens]
    kv_lens = [x[1] for x in seq_lens]

    n_heads, n_kv_heads = num_heads
    assert n_heads % n_kv_heads == 0
    max_kv_len = max(kv_lens)
    sm_scale = head_size**-0.5

    # Generate random query, key, and value tensors.
    query = torch.randn(sum(q_lens), n_heads, head_size, dtype=dtype)
    key_cache = torch.randn(n_blocks, block_size, n_kv_heads, head_size, dtype=dtype)
    value_cache = torch.randn(n_blocks, block_size, n_kv_heads, head_size, dtype=dtype)

    max_n_blocks_per_seq = (max_kv_len + block_size - 1) // block_size
    block_tables = torch.randint(
        0, n_blocks, (n_seqs, max_n_blocks_per_seq), dtype=torch.int32
    )

    # prepare input tensors for the kernel
    qo_indptr = [0]
    kv_indptr = [0]
    kv_indices = []
    kv_last_page_lens = []
    for i in range(n_seqs):
        seq_len = kv_lens[i]
        assert seq_len > 0

        qo_indptr.append(qo_indptr[-1] + q_lens[i])

        num_blocks = (seq_len + block_size - 1) // block_size
        kv_indptr.append(kv_indptr[-1] + num_blocks)
        kv_indices.extend(block_tables[i, :num_blocks])

        kv_last_page_len = seq_len % block_size
        if kv_last_page_len == 0:
            kv_last_page_len = block_size
        kv_last_page_lens.append(kv_last_page_len)

    qo_indptr = torch.tensor(qo_indptr, dtype=torch.int32)
    kv_indptr = torch.tensor(kv_indptr, dtype=torch.int32)
    kv_indices = torch.tensor(kv_indices, dtype=torch.int32)
    kv_last_page_lens = torch.tensor(kv_last_page_lens, dtype=torch.int32)

    wrapper = kernels.BatchPrefillWrapper(False)
    # TODO: determine the best size for the workspace buffer.
    float_workspace_buffer = torch.empty(128 * 1024 * 1024, dtype=torch.int8)
    int_workspace_buffer = torch.empty(8 * 1024 * 1024, dtype=torch.uint8)

    empty_q_data = torch.empty(0, dtype=dtype)

    wrapper.plan(
        float_workspace_buffer,
        int_workspace_buffer,
        qo_indptr,
        kv_indptr,
        n_seqs,
        n_heads,
        n_kv_heads,
        head_size,
        block_size,
        empty_q_data,
    )

    output = wrapper.run(
        query,
        qo_indptr,
        key_cache,
        value_cache,
        kv_indptr,
        kv_indices,
        kv_last_page_lens,
        0,  # pos_encoding_mode
        -1,  # window_left
        logits_soft_cap,
        sm_scale,
    )

    ref_output = varlen_masked_self_attention(
        query=query,
        key_cache=key_cache,
        value_cache=value_cache,
        query_lens=q_lens,
        kv_lens=kv_lens,
        block_tables=block_tables,
        sm_scale=sm_scale,
        logits_soft_cap=logits_soft_cap,
    )

    atol=1e-3 if dtype == torch.float16 else 1e-2
    rtol=1e-3 if dtype == torch.float16 else 1e-2
    torch.testing.assert_close(output, ref_output, atol=atol, rtol=rtol)


if __name__ == "__main__":
    pytest.main([__file__])
