package aparapi.examples.raytrace;

abstract class SceneObject {
	public Surface Surface;

	public abstract ISect Intersect(Ray ray);

	public abstract Vector Normal(Vector pos);
}

class Sphere extends SceneObject {
	public Vector Center;
	public double Radius;

	public Sphere(Vector center, double radius, Surface surface) {
		this.Center = center;
		this.Radius = radius;
		this.Surface = surface;
	}
	
	public ISect Intersect(Ray ray) {
		Vector eo = Vector.Minus(Center, ray.Start);
		double v = Vector.Dot(eo, ray.Dir);
		double dist;
		if (v < 0) {
			return null;
		} else {
			double disc = Math.pow(Radius, 2) - (Vector.Dot(eo, eo) - Math.pow(v, 2));
			dist = disc < 0 ? 0 : v - Math.sqrt(disc);
		}
		
		if(dist==0) return null;
		
		ISect result = new ISect();
		result.Thing = this;
		result.Ray = ray;
		result.Dist = dist;
		
		return result;

	}

	public Vector Normal(Vector pos) {
		return Vector.Norm(Vector.Minus(pos, Center));
	}
}

class Plane extends SceneObject {
	public Vector Norm;
	public double Offset;

	public Plane(Vector norm, double offset, Surface surf) {
		this.Norm = norm;
		this.Offset = offset;
		this.Surface = surf;
	}
	
	public ISect Intersect(Ray ray) {
		double denom = Vector.Dot(Norm, ray.Dir);
		if (denom > 0)
			return null;
		ISect result = new ISect();
		result.Thing = this;
		result.Ray = ray;
		result.Dist = (Vector.Dot(Norm, ray.Start) + Offset) / (-denom);
		return result;
	}

	public Vector Normal(Vector pos) {
		return Norm;
	}
}