package aparapi.examples.raytrace;

abstract class Surface {
    public abstract Color Diffuse(Vector v);
    public abstract Color Specular(Vector v);
    public abstract double Reflect(Vector v);
    public double Roughness;
}
