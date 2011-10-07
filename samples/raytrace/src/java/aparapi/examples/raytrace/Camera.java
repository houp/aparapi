package aparapi.examples.raytrace;

class Camera {

	public Vector Pos;
	public Vector Forward;
	public Vector Up;
	public Vector Right;

	private static final Vector down = new Vector(0,-1,0);
	
	public static Camera Create(Vector pos, Vector lookAt) {
		Vector forward = Vector.Norm(Vector.Minus(lookAt, pos));
		Vector right = Vector.Norm(Vector.Cross(forward, down)).Mult(1.5);
		Vector up = Vector.Norm(Vector.Cross(forward, right)).Mult(1.5);

		Camera c = new Camera();
		c.Pos = pos;
		c.Forward = forward;
		c.Up = up;
		c.Right = right;
		return c;
	}
}