nums = [1, 2, 3, 4, 5]
strs = ["Apple", "Banana", "Coconut"]
spcs = [" Apple", " Banana ", "Coco nut "]

print [(i, c) for i in nums for s in strs if len(s) % 2 != 0 for c in s]
