def is_freq_map_matched(left:dict, right:dict) -> bool:
    if len(left) != len(right):
        return False
    for key, value in left.items():
        if right.get(key) is None:
            return False
        if value != right[key]:
            return False
    return True

def min_substring(input:str, sub_str: str):
    if input is None or len(input) == 0:
        raise ValueError('input')
    if sub_str is None or len(sub_str) == 0:
        raise ValueError('sub_str')
    if len(sub_str) > len(input):
        raise ValueError('sub string cannot be greater than input')
    
    left = 0
    freq_map = {}
    curent_freq_map = {}
    min_window = (0, len(input))

    # build immutable freq_map based on input sub_str
    for c in sub_str:
        if freq_map.get(c) is None:
            freq_map[c] = 1
        else:
            freq_map[c] += 1
    
    for right in range(len(input)):
        # expand window and update curent_freq_map
        cur_char = input[right]
        if freq_map.get(cur_char) is not None:
            if curent_freq_map.get(cur_char) is None:
                curent_freq_map[input[right]] = 1
            else:
                curent_freq_map[cur_char] += 1

        # while window is valid (i.e. curent_freq_map matches freq_map)
        while is_freq_map_matched(freq_map, curent_freq_map):
            cur_char_2 = input[left]
            if curent_freq_map.get(cur_char_2) is not None:
                curent_freq_map[cur_char_2] -= 1
            
            if right - left < min_window[1] - min_window[0]:
                min_window = (left, right)

            left += 1
    return min_window

def main():
    input = 'a11b111c111ab'
    min = min_substring(input, 'abc')
    print(input[min[0]: min[1] + 1])

if __name__ == "__main__":
    main()