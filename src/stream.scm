(define (cons-stream x y)
  
  )

; (head (cons-stream x y)) => x
(define (head s)
  
  )

; (tail (cons-stream x y)) => y
(define (tail s)
  
  )

(define the-empty-stream 'the-empty-stream)

(define (empty-stream? s) (= s the-empty-stream))

(define (map-stream proc s)
  (if (empty-stream? s)
    the-empty-stream
    (cons-stream (proc (head s))
                 (map-stream proc (tail s)))))

(define (filter pred s)
  (cond 
    [(empty-stream? s) the-empty-stream]
    [(pred (head s))
     (cons-steram (head s)
                  (filter pred (tail s)))]
    [else (filter pred (tail s))]))

(define (accumulate combiner init-val s)
  (if (empty-stream? s)
      init-val
      (combiner (head s)
                (accumulate combiner
                            init-val
                            (tail s)))))

(define (append-streams s1 s2)
  (if (empty-stream? s1)
    s2
    (cons-stream (head s1)
                 (append-streams (tails s1)
                                 s2))))

(define (enumerate-tree tree)
  (if (leaf-node? tree)
      (cons-stream tree
                   the-empty-stream)
      (append-streams
        (enumerate-tree (left-branch tree))
        (enumerate-tree (right-branch tree)))))

(define (enum-interval low high)
  (if (= low high)
      the-empty-stream
      (cons-stream low
                   (enum-interval (inc low) high))))

(define (odd-fibs n)
  (accumulate
    cons
    '()
    (filter
      odd
      (map fib (enum-interval 1 n)))))

