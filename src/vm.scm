(load "lib.scm")

(define (lookup var e)
  (recur nxtrib ([e e])
     (recur nxtelt ([vars (caar e)] [vals (cdar e)])
       (cond
         [(null? vars)
          (let ([next (cdr e)])
            (if (null? next)
                (error "lookup: no such binding" var)
                (nxtrib next)))]
         [(eq? (car vars) var) vals]
         [else (nxtelt (cdr vars) (cdr vals))]))))

(define (closure body e vars)
  (list body e vars))

(define (continuation s)
  (closure (list 'nuate s 'v) '() '(v)))

(define (call-frame x e r s)
  (list x e r s))

(define (extend e vars vals)
  (cons (cons vars vals) e))

; vm registers
; a: the accumulator
; x: the next expression
; e: the current environment
; r: the current value rib
; s: the current stack

(define (VM a x e r s)
  #;(for-each display
    (list
      (list 'a a)
      (list 'x x)
      (list 'e e)
      (list 'r r)
      (list 's s)
      'call-frame))
  (record-case x
    [halt () a]
    [refer (var x)
      (VM (car (lookup var e)) x e r s)]
    [constant (obj x)
      (VM obj x e r s)]
    [close (vars body x)
      (VM (closure body e vars) x e r s)]
    [test (then else)
      (VM a (if a then else) e r s)]
    [assign (var x)
      (begin
        (set-car! (lookup var e) a)
        (VM a x e r s))]
    [conti (x)
      (VM (continuation s) x e r s)]
    [nuate (s var)
      (VM (car (lookup var e)) '(return) e r s)]
    [frame (ret x)
      (VM a x e '() (call-frame ret e r s))]
    [argument (x)
      (VM a x e (cons a r) s)]
    [apply ()
      (if (procedure? a)
        (VM (apply a r) '(return) e '() s)
        (record a (body e vars)
          (VM a body (extend e vars r) '() s)))]
    [return ()
      (record s (x e r s)
        (VM a x e r s))]))

