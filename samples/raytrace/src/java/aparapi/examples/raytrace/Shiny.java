package aparapi.examples.raytrace;

public class Shiny extends Surface {
	private static final Shiny _instance = new Shiny();

	public static Shiny getInstance() {
		return _instance;
	}

	public Shiny() {
		Roughness = 50;

	}

	@Override
	public Color Diffuse(Vector v) {
		return Color.Make(1, 1, 1);
	}

	@Override
	public Color Specular(Vector v) {
		return Color.Make(.5, .5, .5);
	}

	@Override
	public double Reflect(Vector v) {
		return .6;
	}

}