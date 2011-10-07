package aparapi.examples.raytrace;

public class Color {
	public static final Color Black = new Color(0,0,0);
	public static final Color White = new Color(1,1,1);
	public static final Color Gray = new Color(.5,.5,.5);

	public double R;
	public double G;
	public double B;

	public Color(double r, double g, double b) {
		R = r;
		G = g;
		B = b;
	}

	public static Color Make(double r, double g, double b) {
		return new Color(r, g, b);
	}

	public static Color Times(double n, Color v) {
		return new Color(n * v.R, n * v.G, n * v.B);
	}

	public static Color Times(Color v1, Color v2) {
		return new Color(v1.R * v2.R, v1.G * v2.G, v1.B * v2.B);
	}

	public static Color Plus(Color v1, Color v2) {
		return new Color(v1.R + v2.R, v1.G + v2.G, v1.B + v2.B);
	}

	public static Color Minus(Color v1, Color v2) {
		return new Color(v1.R - v2.R, v1.G - v2.G, v1.B - v2.B);
	}

	public double Legalize(double d) {
		return d > 1 ? 1 : d;
	}

	public int GetPacked() {
		return ((int) (R * 255) * 65536) + ((int) (G * 255) * 256)
				+ ((int) (B * 255));
	}
}