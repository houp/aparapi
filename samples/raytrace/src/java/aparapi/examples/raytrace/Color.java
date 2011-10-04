package aparapi.examples.raytrace;

public class Color {
	public double R;
	public double G;
	public double B;

	public Color(double r, double g, double b) {
		R = r;
		G = g;
		B = b;
	}

	public Color(String str) throws Exception {
		String[] nums = str.split(",");
		if (nums.length != 3)
			throw new Exception();
		R = Double.parseDouble(nums[0]);
		G = Double.parseDouble(nums[1]);
		B = Double.parseDouble(nums[2]);
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

	public static final Color Background = Make(0, 0, 0);
	public static final Color DefaultColor = Make(0, 0, 0);

	public double Legalize(double d) {
		return d > 1 ? 1 : d;
	}
}