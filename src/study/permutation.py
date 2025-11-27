from typing import List
def internal_perm(nums:List[int]) -> List[list[int]]:
    if len(nums) == 1:
        return [[nums[0]]]
    
    result: List[List[int]] = []
    for i in range(len(nums)):
        cur = nums.pop(0)
        rs = internal_perm(nums)
        for r in rs:
            r.append(cur)
            result.append(r)
        nums.append(cur)
    return result

def main():
    print(internal_perm([1,2,3]))

if __name__ == "__main__":
    main()
