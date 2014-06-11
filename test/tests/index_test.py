def reverse(self):
    'S.reverse() -- reverse *IN PLACE*'
    n = len(self)
    for i in range(n//2):
        self[i], self[n-i-1] = self[n-i-1], self[i]

