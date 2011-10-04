package aparapi.examples.raytrace;

public class CheckerBoard extends Surface {
	private static final CheckerBoard _instance = new CheckerBoard();

	
	public static CheckerBoard getInstance() {
		return _instance;
	}
	
	public CheckerBoard() {
		Roughness = 150;
	}

	@Override
	public Color Diffuse(Vector pos) {
		return ((Math.floor(pos.Z) + Math.floor(pos.X)) % 2 != 0) ? Color.Make(
				1, 1, 1) : Color.Make(0, 0, 0);
	}

	@Override
	public Color Specular(Vector pos) {
		return Color.Make(1, 1, 1);
	}

	@Override
	public double Reflect(Vector pos) {
		return ((Math.floor(pos.Z) + Math.floor(pos.X)) % 2 != 0) ? .1 : .7;
	}

}
