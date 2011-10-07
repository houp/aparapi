This is a simple raytracer based on C# code by Luke Hoban:
http://blogs.msdn.com/b/lukeh/archive/2007/04/03/a-ray-tracer-in-c-3-0.aspx

Please note that currently this sample DOES NOT work correctly with Aprapi -
OpenCL code can't be generated due to:

- usage of static methods
- usage of "new" operator in the kernel
- some other minor limitations...

So for now it is only possible to run this example in SEQ or JTP modes, but
there is ongoing effort to add missing features to Aparapi so that this
example could run on GPU.

