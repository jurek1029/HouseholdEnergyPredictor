from ctypes import CDLL, POINTER, byref, c_void_p, c_size_t, c_int


cu = CDLL('libcuda.so')

cuInit = cu.cuInit
cuDeviceGet = cu.cuDeviceGet
cuCtxCreate = cu.cuCtxCreate

cuMemAlloc = cu.cuMemAlloc
cuMemAlloc.argtypes = [POINTER(c_size_t), c_size_t]

cuMemFree = cu.cuMemFree
cuMemFree.argtypes = [c_void_p]

cuMemGetInfo = cu.cuMemGetInfo
cuMemGetInfo.argtypes = [POINTER(c_size_t), POINTER(c_size_t)]


# Python functions for allocation, deallocation, and memory info

def my_init():
    ret = cuInit(0)
    if ret:
        raise RuntimeError(f'Unexpected return code {ret} from cuInit')
    dev = c_int()
    ret = cuDeviceGet(byref(dev), 0)
    if ret:
        raise RuntimeError(f'Unexpected return code {ret} from cuDeviceGet')
    ctx = c_void_p()
    ret = cuCtxCreate(byref(ctx), 0, dev)
    if ret:
        raise RuntimeError(f'Unexpected return code {ret} from cuCtxCreate')


def my_alloc(size):
    """
    Allocate `size` bytes of device memory and return a device pointer to the
    allocated memory.
    """
    ptr = c_size_t()
    ret = cuMemAlloc(byref(ptr), size)
    if ret:
        raise RuntimeError(f'Unexpected return code {ret} from cuMemAlloc')
    return ptr


def my_free(ptr):
    """
    Free device memory pointed to by `ptr`.
    """
    cuMemFree(ptr)


def my_memory_info():
    free = c_size_t()
    total = c_size_t()
    cuMemGetInfo(byref(free), byref(total))
    return free, total


my_init()
print(my_memory_info())
arr = my_alloc(16384)
print(arr)
print(my_memory_info())
my_free(arr.value)
print(my_memory_info())


from numba import cuda
cuda.detect()