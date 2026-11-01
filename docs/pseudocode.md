# Pseudocode

Pseudocode for every operation of the Airport Code Lookup Service, written before the C++ code.
`root` is the root of the tree, each node has `key` (the IATA code), `data` (the airport record),
`left` and `right`. `h` is the height of the tree and `n` the number of airports.

Insert and search walk down with a `while` loop instead of recursion: building from sorted data
gives a chain of 6,072 nodes, and recursion that deep could overflow the call stack.

---

## Task 1 — Load the data and build the tree

### SPLIT-CSV-LINE(line)

```
1:  SPLIT-CSV-LINE(line)
2:      fields ← empty list;  field ← ""
3:      inQuotes ← false
4:      for i ← 0 to length(line) − 1 do
5:          c ← line[i]
6:          if inQuotes then
7:              if c = '"' and line[i + 1] = '"' then      // "" inside quotes is one quote
8:                  field ← field + '"';  i ← i + 1
9:              else if c = '"' then
10:                 inQuotes ← false                       // closing quote
11:             else
12:                 field ← field + c                      // a comma here belongs to the name
13:         else if c = '"' then
14:             inQuotes ← true
15:         else if c = ',' then
16:             append field to fields;  field ← ""        // end of one field
17:         else
18:             field ← field + c
19:     append field to fields
20:     return fields
```

### LOAD(fileName)

```
1:  LOAD(fileName)
2:      records ← empty array;  skipped ← 0
3:      for each line in fileName do
4:          f ← SPLIT-CSV-LINE(line)
5:          if f[4] = "\N" then                            // no IATA code
6:              skipped ← skipped + 1
7:          else
8:              append Airport(name f[1], city f[2], country f[3], IATA f[4],
                              ICAO f[5], latitude f[6], longitude f[7]) to records
9:      return records, skipped
```

### INSERT(a)  — rules I1 to I4

```
1:  INSERT(a)
2:      if root = NULL then                     // I1: empty tree
3:          root ← new node(a);  return true
4:      cur ← root
5:      while true do
6:          if a.key < cur.key then             // I2: go left
7:              if cur.left = NULL then         // I1: empty spot
8:                  cur.left ← new node(a);  return true
9:              cur ← cur.left
10:         else if a.key > cur.key then        // I3: go right
11:             if cur.right = NULL then        // I1: empty spot
12:                 cur.right ← new node(a);  return true
13:             cur ← cur.right
14:         else                                // I4: duplicate, do nothing
15:             return false
```

### BUILD(records)

```
1:  BUILD(records)
2:      root ← NULL;  duplicates ← 0
3:      for i ← 0 to n − 1 do
4:          if not INSERT(records[i]) then
5:              duplicates ← duplicates + 1
6:              print "duplicate", records[i].key
7:      return duplicates
```

### HEIGHT and AVERAGE-DEPTH (level order)

```
1:  LEVELS(root)
2:      result ← empty list;  level ← [root]
3:      while level is not empty do
4:          append the keys of level to result
5:          next ← empty list
6:          for i ← 0 to size(level) − 1 do
7:              if level[i].left ≠ NULL then append level[i].left to next
8:              if level[i].right ≠ NULL then append level[i].right to next
9:          level ← next
10:     return result

11: HEIGHT()
12:     return size(LEVELS(root)) − 1           // −1 for an empty tree

13: AVERAGE-DEPTH()
14:     L ← LEVELS(root);  total ← 0
15:     for d ← 0 to size(L) − 1 do
16:         total ← total + d × size(L[d])      // every node on level d has depth d
17:     return total / n
```

Cost: BUILD is Θ(n · h) in total (each insert walks one path); LEVELS is Θ(n).

---

## Task 2 — Search

### FIND(code)  — rules S1 to S4

```
1:  FIND(code)
2:      path ← empty list
3:      cur ← root
4:      while cur ≠ NULL do
5:          append cur.key to path
6:          if code = cur.key then return cur.data, path       // S2: found
7:          else if code < cur.key then cur ← cur.left         // S3: go left
8:          else cur ← cur.right                               // S4: go right
9:      return NOT FOUND, path                                 // S1: reached NULL
```

### FIND-MIN and FIND-MAX

```
1:  FIND-MIN()                          1:  FIND-MAX()
2:      cur ← root                      2:      cur ← root
3:      while cur.left ≠ NULL do        3:      while cur.right ≠ NULL do
4:          cur ← cur.left              4:          cur ← cur.right
5:      return cur.data                 5:      return cur.data
```

Cost: Θ(h) in the worst case (h + 1 comparisons), Θ(1) in the best case (the root).

---

## Task 3 — Delete

### REMOVE(code)  — rules D1 to D3, the root included

```
1:  REMOVE(code)
2:      parent ← NULL;  cur ← root                         // search, remembering the parent
3:      while cur ≠ NULL and cur.key ≠ code do
4:          parent ← cur
5:          if code < cur.key then cur ← cur.left else cur ← cur.right
6:      if cur = NULL then
7:          return "not found"                             // S1: nothing changes
8:      if cur.left ≠ NULL and cur.right ≠ NULL then       // D3: two children
9:          succParent ← cur;  succ ← cur.right            // right once
10:         while succ.left ≠ NULL do                      // then left as far as possible
11:             succParent ← succ;  succ ← succ.left
12:         cur.data ← succ.data                           // copy the successor in
13:         if succParent = cur then succParent.right ← succ.right
14:         else succParent.left ← succ.right              // succ has no left child: D1 or D2
15:         free succ
16:     else                                               // D1 leaf or D2 one child
17:         child ← (cur.left ≠ NULL) ? cur.left : cur.right   // NULL for a leaf
18:         if parent = NULL then root ← child             // the root pointer itself changes
19:         else if parent.left = cur then parent.left ← child
20:         else parent.right ← child
21:         free cur
22:     return the case that was used
```

### ADD(code, name, city, country)

```
1:  ADD(code, name, city, country)
2:      a ← Airport(code, name, city, country)
3:      if INSERT(a) then print "added" else print "already in the tree (I4)"
4:      print VALIDATE()
```

Cost: Θ(h) — one search down the tree, plus at most one more walk down to the successor.

---

## Task 4 — Traversals and validation

### LIST (inorder, with an explicit stack)

```
1:  LIST()
2:      stack ← empty;  cur ← root
3:      while cur ≠ NULL or stack is not empty do
4:          while cur ≠ NULL do                 // go left as far as possible
5:              push cur;  cur ← cur.left
6:          node ← pop
7:          print node.data                     // visit
8:          cur ← node.right                    // then the right subtree
```

### SAVE(fileName) — preorder

```
1:  SAVE(fileName)
2:      stack ← [root]
3:      while stack is not empty do
4:          node ← pop
5:          write node.key to fileName           // node first
6:          if node.right ≠ NULL then push node.right   // pushed first, visited last
7:          if node.left ≠ NULL then push node.left
```

### LOAD-PREORDER(fileName)

```
1:  LOAD-PREORDER(fileName)
2:      root ← NULL
3:      for each code in fileName do            // codes are in preorder
4:          INSERT(record with this code)
```

Inserting a preorder list into an empty BST rebuilds exactly the same tree: every node is
inserted before all of its descendants, and the BST rules send each key to its old position.

### VALIDATE() — the range (lo, hi) method

```
1:  VALIDATE()
2:      stack ← [(root, −∞, +∞)]
3:      while stack is not empty do
4:          (node, lo, hi) ← pop
5:          if not (lo < node.key < hi) then return false
6:          if node.left ≠ NULL then push (node.left, lo, node.key)     // left: new upper bound
7:          if node.right ≠ NULL then push (node.right, node.key, hi)   // right: new lower bound
8:      return true
```

Cost: LIST, SAVE, LOAD-PREORDER's traversal and VALIDATE are Θ(n).

---

## Task 5 — Range query

### RANGE(from, to)

```
1:  RANGE(from, to)
2:      result ← empty;  visited ← 0
3:      stack ← empty;  cur ← root
4:      while cur ≠ NULL or stack is not empty do
5:          while cur ≠ NULL do
6:              visited ← visited + 1;  push cur
7:              if from < cur.key then cur ← cur.left      // left subtree may hold keys ≥ from
8:              else cur ← NULL                            // everything on the left is < from: skip
9:          node ← pop
10:         if from ≤ node.key ≤ to then append node.data to result
11:         if node.key < to then cur ← node.right         // right subtree may hold keys ≤ to
12:         else cur ← NULL                                // everything on the right is > to: skip
13:     return result, visited
```

Cost: Θ(h + k), where k is the number of airports in the range.

---

## Task 6 — Balanced build (the `rebalance` command)

### BUILD-BALANCED(sorted, lo, hi)

```
1:  BUILD-BALANCED(sorted, lo, hi)                // sorted by code
2:      if lo > hi then return NULL
3:      mid ← lo + (hi − lo) / 2                  // middle element becomes the root
4:      node ← new node(sorted[mid])
5:      node.left ← BUILD-BALANCED(sorted, lo, mid − 1)
6:      node.right ← BUILD-BALANCED(sorted, mid + 1, hi)
7:      return node

8:  REBALANCE()
9:      sorted ← LIST()                           // inorder gives the keys in order
10:     root ← BUILD-BALANCED(sorted, 0, n − 1)
```

The recursion depth is only about log₂ n (13 for 6,072 keys), so recursion is safe here.
Cost: Θ(n). The result has height ⌊log₂ n⌋ = 12.

### TIME-SEARCHES(tree, targets)

```
1:  TIME-SEARCHES(tree, targets)                 // 10,000 random codes from the data
2:      start ← clock()
3:      for i ← 0 to 9999 do
4:          FIND(targets[i])
5:      return clock() − start
```

---

## Summary of costs

| Operation | Best case | Worst case |
|---|---|---|
| insert, find, remove | Θ(1) | Θ(h) |
| min, max | Θ(1) | Θ(h) |
| range | Θ(h + k) | Θ(h + k) |
| list, save, validate, height | Θ(n) | Θ(n) |
| build by repeated insertion | Θ(n log n) | Θ(n²) (sorted input, h = n − 1) |
| balanced build | Θ(n) | Θ(n) |

For this data set h = 177 in file order, 6,071 in sorted order and 12 after `rebalance`.
