from typing import List
def comb(nums:list[int])->List[List[int]]:
    if len(nums) == 1:
        return[[nums[0]]]

    cur = nums.pop(0)
    result : List[List[int]] = []

    result.append([cur])
    rs = comb(nums)
    for r in rs:
        result.append(r)
        new_list = r + [cur]
        result.append(new_list)
        
    return result

def main():
    print('result', comb([1,2,3]))

if __name__ == "__main__":
    main()





