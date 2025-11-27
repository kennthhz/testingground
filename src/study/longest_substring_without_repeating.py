def longest_substring_without_repeating(input:str):
    seen = {}
    longest_length = 1
    longest_pair = (0,0)
    left = 0
    for right in range(len(input)):
        cur_char = input[right]
        if seen.get(cur_char) is None:
            seen[input[right]] = 1
        else:
            dup_char = cur_char
            while input[left] != dup_char:
                seen.pop(input[left])
                left += 1
            left += 1
        if right - left + 1 > longest_length:
            longest_length = right - left + 1
            longest_pair = (left, right)
    
    return longest_pair

def main():
    input = "dvdf"
    result = longest_substring_without_repeating(input)
    print(result)
    print(input[result[0]:result[1] + 1])

if __name__ == "__main__":
    main()



