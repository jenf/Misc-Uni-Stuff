>module OBfi (test) where

This can probably be improved somewhat, I can't
truely be bothered.

>import IO
>import IOExts
>import Array
>import Char
>data BF = Nil | Add Int BF | Sub Int BF | IncP Int BF | DecP Int BF | Put BF | Get BF | Start BF BF deriving Show

>start = newIOArray (0::Int,50000::Int) (0::Int)

>compile [] = Nil
>compile (x:y) | x=='+' = Add 1 $ compile y
>              | x=='-' = Sub 1 $ compile y
>              | x=='>' = IncP 1 $ compile y
>              | x=='<' = DecP 1 $ compile y
>              | x=='[' = do_start y
>              | x=='.' = Put $ compile y
>              | x==',' = Get $ compile y 
>              | otherwise = compile y

>do_start x = (Start (compile g) (compile f))
>              where (g,f)=findend x 1

>findend :: String -> Int -> (String,String)
>findend [] y = ([],[])
>findend (x:xs) y | x=='['        = let (j,k) = findend xs (y+1) in (x:j,k)
>                 | x==']' && y==1= ([],xs)
>                 | x==']'        = let (l,m) = findend xs (y-1) in (x:l,m)
>                 | otherwise     = let (g,h) = findend xs y in  (x:g,h)

>reduce (Add x (Add y g))=(reduce (Add (x+y) g))
>reduce (Sub x (Sub y g))=(reduce (Sub (x+y) g))
>reduce (Add x y)=Add x (reduce y)
>reduce (Sub x y)=Sub x (reduce y)
>reduce (IncP x (IncP y g))=reduce (IncP (x+y) g)
>reduce (DecP x (DecP y g))=reduce (DecP (x+y) g)
>reduce (IncP x y)=IncP x (reduce y)
>reduce (DecP x y)=DecP x (reduce y)
>reduce (Put y)=Put (reduce y)
>reduce (Get y)=Get (reduce y)
>reduce (Start x y)=Start (reduce x) (reduce y)
>reduce Nil = Nil  

>test :: String -> IO(Int,IOArray Int Int)
>test x = do
>          g <- start
>          proc (reduce(compile x)) (0,g)

>test_ x = do
>          f <- readFile x
>          g <- start
>          print $ reduce(compile f)

>proc :: BF -> (Int,IOArray Int Int) -> IO (Int,IOArray Int Int)
>proc Nil p = return p
>proc (Add x xs) w@(p,g) = do
>                          h <- readIOArray g p
>                          writeIOArray g p (h+x)
>                          proc xs w
>proc (Sub x xs) w@(p,g) = do
>                          h <- readIOArray g p
>                          writeIOArray g p (h-x)
>                          proc xs w
>proc (IncP x xs) w@(p,g) | (p+x)>50000 = error "OVER RANGE"
>                         | otherwise = proc xs (p+x,g)
>proc (DecP x xs) w@(p,g) | (p-x)<0     = error "OVER RANGE"
>                         | otherwise = proc xs (p-x,g)
>proc (Put xs) w@(p,g)    = do
>                           h <- readIOArray g p
>                           putChar (wrap_chr h)
>                           proc xs w
>proc (Get xs) w@(p,g)    = do
>                           h <- get_char 
>                           writeIOArray g p (ord h)
>                           proc xs w 
>proc (Start x xs) w@(p,g) = proc_while x xs w

>proc_while x y (p,g) = do
>                       h <- readIOArray g p
>                       if ((wraper h)==0) then proc y (p,g) else proc_next x y (p,g)

>proc_next x y (p,a) = do
>                      f <- proc x (p,a)
>                      proc_while x y f

>wrap_pointer x = abs (x `mod` 30000)
>wraper x = abs (x `mod` 256)
>wrap_chr x = chr $ abs (x `mod` 256)

>get_char = do
>            l <- isEOF
>            if l then return (chr 0) else getChar



