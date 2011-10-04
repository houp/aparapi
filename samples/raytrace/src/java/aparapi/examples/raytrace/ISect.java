package aparapi.examples.raytrace;

class ISect implements Comparable<ISect> {
    public SceneObject Thing;
    public Ray Ray;
    public double Dist;
	@Override
	public int compareTo(ISect o) {
		return Double.compare(Dist, o.Dist);
	}
}
