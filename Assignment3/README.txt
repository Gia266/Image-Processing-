Gia Lee - 19jl253 - 20231785
William Robson - 19wsar - 20220841

Note: 
Weird error with hough.h not freeing counts[] properly when program closes
I don't think it was anything we did, causes crash from bus error
Easiest fix is to close window properly to trigger ~hough and retry