(write (case 'hello
         [(1) 2]
         [else 'hello]))

(write (case (+ 7 5)
         [(1 2 3) 'small]
         [(10 11 12) 'big]))

(write (case (- 7 5)
         [(1 2 3) 'small]
         [(10 11 12) 'big]))

(write (case '(y x)
         [((a b) (x y)) 'forwards]
         [((b a) (y x)) 'backwards]))

(write (case 'x
         [(x) "ex"]
         [('x) "quoted ex"]))
