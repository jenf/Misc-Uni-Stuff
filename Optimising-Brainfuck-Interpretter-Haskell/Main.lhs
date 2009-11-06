>module Main where
>import System
>import OBfi
>import IO
>main = do
>         argv <- getArgs
>         h <- readFile $ head argv
>         hSetBuffering stdout NoBuffering
>         test h 
