(define vec #(1 2 3))
(write vec)
(write (vector? 1))
(write (vector? "hello"))
(write (vector? '(1 2 3)))
(write (vector? vec))
(write (vector-length vec))
(write (vector-ref vec 0))
(write (vector-ref vec 1))
(write (vector-ref vec 2))
(vector-set! vec 0 4)
(vector-set! vec 1 5)
(vector-set! vec 2 6)
(write vec)
(write (vector-length vec))
(write (vector-ref vec 0))
(write (vector-ref vec 1))
(write (vector-ref vec 2))
