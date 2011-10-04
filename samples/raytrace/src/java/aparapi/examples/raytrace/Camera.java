package aparapi.examples.raytrace;

class Camera {

	public Vector Pos;
	public Vector Forward;
	public Vector Up;
	public Vector Right;

	public static Camera Create(Vector pos, Vector lookAt) {
		Vector forward = Vector.Norm(Vector.Minus(lookAt, pos));
		Vector down = new Vector(0, -1, 0);
		Vector right = Vector.Times(1.5,
				Vector.Norm(Vector.Cross(forward, down)));
		Vector up = Vector
				.Times(1.5, Vector.Norm(Vector.Cross(forward, right)));

		Camera c = new Camera();
		c.Pos = pos;
		c.Forward = forward;
		c.Up = up;
		c.Right = right;
		return c;
	}
}