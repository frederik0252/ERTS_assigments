# GEMM


GEMM run 1: 111.418 ms
GEMM run 2: 111.440 ms
GEMM run 3: 111.429 ms
GEMM run 4: 111.418 ms
GEMM run 5: 111.415 ms
GEMM run 6: 111.413 ms
GEMM run 7: 111.409 ms
GEMM run 8: 111.409 ms
GEMM run 9: 111.402 ms
GEMM run 10: 111.407 ms

GEMM Summary (M=256, K=192, N=192, bias=true):
  Median latency:           111.414 ms
  Approx buffer footprint:  528.750 KiB
  Sample output[0]:         19.587595
  MACs:                     9437184
  Total buffer footprint:   528.750000 KiB
  Per-frame I/O (w cached): 384.000000 KiB



## layerNorm

LayerNorm run 1: 7.675 ms
LayerNorm run 2: 7.676 ms
LayerNorm run 3: 7.675 ms
LayerNorm run 4: 7.676 ms
LayerNorm run 5: 7.674 ms
LayerNorm run 6: 7.676 ms
LayerNorm run 7: 7.675 ms
LayerNorm run 8: 7.675 ms
LayerNorm run 9: 7.676 ms
LayerNorm run 10: 7.675 ms

LayerNorm Summary (M=256, D=192, affine=true):
  Median latency:             7.675 ms
  Approx arithmetic ops:      246784
  Input buffer:               192.00 KiB
  Output buffer:              192.00 KiB
  Params (gamma+beta):        1.50 KiB
  Per-frame I/O (in+out):     384.00 KiB
  Total buffer footprint:     385.50 KiB (394752 bytes)
  Effective buffer BW (SW):   49.05 MiB/s
  Sample output[0]:           -1.570197
