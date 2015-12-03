Dig Code

Make code as a SNAPSHOT.
After code modified, we can get a different SNAPSHOT. With the compasion, we can find changes.
What we exactly get can help us build the map what really changed: a method added/modified/removed? 
Some function invoking added/removed/modified?

With the SNAPSHOT, we can build the program revolution map: what we changed? what's the code hot-point? 
If it's a bug-fix, can we make the RULE and find more similar issues?

This project is part of SNAPSHOT maker. 
We mainly focus on C-like language in this repo -- as you saw we adopted LLVM+CLANG as the analyzing engine.
