package aparapi.examples.raytrace;

import java.util.*;

class Scene {
	public SceneObject[] Things;
	public Light[] Lights;
	public Camera Camera;

	public Enumeration<ISect> Intersect(Ray r) {
		List<ISect> result = new ArrayList<ISect>();
		for (SceneObject thing : Things) {
			result.add(thing.Intersect(r));
		}
		return Collections.enumeration(result);

	}
}
