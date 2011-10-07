package aparapi.examples.raytrace;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

import javax.swing.JComponent;
import javax.swing.JFrame;

public class Main {

	public static void main(String[] _args) {

		JFrame frame = new JFrame("Raytracer");

		final int width = 750;

		final int height = 750;

		final BufferedImage image = new BufferedImage(width, height,
				BufferedImage.TYPE_INT_RGB);

		@SuppressWarnings("serial")
		JComponent viewer = new JComponent() {
			@Override
			public void paintComponent(Graphics g) {

				g.drawImage(image, 0, 0, width, height, this);
			}
		};

		viewer.setPreferredSize(new Dimension(width, height));

		// Swing housework to create the frame
		frame.getContentPane().add(viewer);
		frame.pack();
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);

		// Extract the underlying RGB buffer from the image.
		// Pass this to the kernel so it operates directly on the RGB buffer of
		// the image
		final int[] imageRgb = ((DataBufferInt) image.getRaster()
				.getDataBuffer()).getData();

		Raytracer rt = new Raytracer(width, height, imageRgb);
		long t1 = System.currentTimeMillis();
		
		rt.execute(width*height);
		long t2 = System.currentTimeMillis();
		System.out.println("Time: "+(t2-t1)+" ms.");
		
		viewer.repaint();

		// Window listener to dispose Kernel resources on user exit.
		frame.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent _windowEvent) {
				System.exit(0);
			}
		});

	}

}
