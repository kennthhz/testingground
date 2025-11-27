def max_subarray_sum(data:list[int],size:int):
    if size > len(data):
        raise ValueError('size > data len')
    sum = 0
    max_sum = -float('inf')
    left = 0
    for right in range(len(data)):
        sum += data[right]

        if (right - left + 1 > size):
            sum -= data[left]
            left += 1
        
        if (right - left + 1 == size):
            if sum > max_sum:
                max_sum = sum
    return max_sum








