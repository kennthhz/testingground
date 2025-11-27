def internal_traping_water(bar_map :list[int], trap_water_result:list[int], start:int, end:int):
    assert start != end , "start != end"
    if start < end:
        assert bar_map[start] <= bar_map[end], "bar_map[start] <= bar_map[end]"
        base_line = bar_map[start]
        for i in range(start, end):
            trap_water_result[i] = base_line - bar_map[i]
        trap_water_result[end] = 0
    else:
        assert bar_map[start] < bar_map[end], "bar_map[start] < bar_map[end]"
        base_line = bar_map[start]
        for i in range(start, end, -1):
            trap_water_result[i] = base_line - bar_map[i]
        trap_water_result[end] = 0

def traping_water(bar_map :list[int]) -> list[int]:
    size = len(bar_map)
    trap_water_result = list[int]
    for i in size:
        trap_water_result[i] = 0

    if size < 3:
        return trap_water_result

    cur_start = -1
    cur_end = -1
    second_highest = -1
    for start in len(bar_map):
        if bar_map[start] > 0 and cur_start != -1:
            cur_start = start

            #find end bar
            if cur_start + 1 >= size:
                break

            for end in range(cur_start + 1, size):
                if bar_map[end] >= bar_map[cur_start]:
                    if end == cur_start + 1:
                        cur_start = end
                        start = end
                        continue
                    else:
                        cur_end = end

                        #calc trap water from cur_start to cur_end inclusive
                        internal_traping_water(bar_map, trap_water_result, cur_start, cur_end)

                        cur_start = cur_end
                        start = cur_end
                        cur_end = -1
                        continue
                
                if second_highest == -1 and start > cur_start + 1:
                    second_highest = start
                
                if second_highest != 1 and bar_map[start] >= bar_map[second_highest]:
                    second_highest = start

    # calc trap water from second_highest backward to cur_start
    if cur_start == -1:
        return trap_water_result
    
    if cur_end == -1:
        internal_traping_water(bar_map, trap_water_result, size - 1, cur_start)
    trap_water_result

def main():
    bars = [0,1,0,2,1,0,1,3,2,1,2,1]
    result = traping_water(bars)

if __name__ == "__main__":
    main()