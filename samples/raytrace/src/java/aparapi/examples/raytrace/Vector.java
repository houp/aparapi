package aparapi.examples.raytrace;

class Vector {
    public double X;
    public double Y;
    public double Z;

    public Vector(double x, double y, double z) { X = x; Y = y; Z = z; }
    public Vector(String str) throws Exception {
        String[] nums = str.split(",");
        if (nums.length != 3) throw new Exception();
        X = Double.parseDouble(nums[0]);
        Y = Double.parseDouble(nums[1]);
        Z = Double.parseDouble(nums[2]);
    }
    public static Vector Make(double x, double y, double z) { return new Vector(x, y, z); }
    public static Vector Times(double n, Vector v) {
        return new Vector(v.X * n, v.Y * n, v.Z * n);
    }
    public static Vector Minus(Vector v1, Vector v2) {
        return new Vector(v1.X - v2.X, v1.Y - v2.Y, v1.Z - v2.Z);
    }
    public static Vector Plus(Vector v1, Vector v2) {
        return new Vector(v1.X + v2.X, v1.Y + v2.Y, v1.Z + v2.Z);
    }
    public static double Dot(Vector v1, Vector v2) {
        return (v1.X * v2.X) + (v1.Y * v2.Y) + (v1.Z * v2.Z);
    }
    public static double Mag(Vector v) { return Math.sqrt(Dot(v, v)); }
    public static Vector Norm(Vector v) {
        double mag = Mag(v);
        double div = mag == 0 ? Double.POSITIVE_INFINITY : 1 / mag;
        return Times(div, v);
    }
    public static Vector Cross(Vector v1, Vector v2) {
        return new Vector(((v1.Y * v2.Z) - (v1.Z * v2.Y)),
                          ((v1.Z * v2.X) - (v1.X * v2.Z)),
                          ((v1.X * v2.Y) - (v1.Y * v2.X)));
    }
    public static boolean Equals(Vector v1, Vector v2) {
        return (v1.X == v2.X) && (v1.Y == v2.Y) && (v1.Z == v2.Z);
    }
}