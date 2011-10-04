This is a simple raytracer based on C# code by Luke Hoban:
http://blogs.msdn.com/b/lukeh/archive/2007/04/03/a-ray-tracer-in-c-3-0.aspx

Please note that currently this sample DOES NOT work correctly with Aprapi -
OpenCL code can't be generated due to:

- usage of static methods
- usage of "new" operator in the kernel
- usgae of Java collections

While the third problem can be easily eliminated by rewriting the code to use
plain arrays, the first two need a bit more work. I hope to add some raw 
support for static calls into Aparapi core, and think about "new" support
in some way.

So for now it is only possible to run this example in SEQ or JTP modes.

