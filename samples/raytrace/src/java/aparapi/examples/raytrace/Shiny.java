package aparapi.examples.raytrace;

public class Shiny extends Surface {
	private static final Shiny _instance = new Shiny();

	public static Shiny getInstance() {
		return _instance;
	}

	private Color difuse;
	private Color specular;
	private double reflect;
	
	public Shiny() {
		Roughness = 50;
		difuse = Color.White;
		specular = Color.Gray;
		reflect = .6;

	}

	@Override
	public Color Diffuse(Vector v) {
		return difuse;
	}

	@Override
	public Color Specular(Vector v) {
		return specular;
	}

	@Override
	public double Reflect(Vector v) {
		return reflect;
	}

}