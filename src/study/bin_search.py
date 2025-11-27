from typing import List
def binSearch(nums:List[int], target:int) -> List[int]:
    if target < nums[0]:
        return [0,-1]
    if target > nums[len(nums)-1]:
        return [len(nums), len(nums)-1]
    
    low = 0
    high = len(nums)-1
    while (low <= high):
        mid = int((low + high) / 2)
        if target == nums[mid]:
            return [mid, mid]
        elif (target > nums[mid]):
            low = mid + 1
        else:
            high = mid - 1
    
    print (f"low:{low} high:{high}")
    return [low, high]


input = [1,8,10]
r = binSearch(input,7)
print(r)

