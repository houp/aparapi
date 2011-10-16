package aparapi.examples;

import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;

import javax.swing.JComponent;
import javax.swing.JFrame;

public class Main {

	public static void main(String[] _args) {

		JFrame frame = new JFrame("Game of Life");

		final int width = 500;

		final int height = width;

	
		final BufferedImage image = new BufferedImage(width, height,
				BufferedImage.TYPE_BYTE_GRAY);

		@SuppressWarnings("serial")
		JComponent viewer = new JComponent() {
			@Override
			public void paintComponent(Graphics g) {

				g.drawImage(image, 0, 0, width, height, this);
			}
		};

		// Set the size of JComponent which displays Mandelbrot image
		viewer.setPreferredSize(new Dimension(width, height));

		// Swing housework to create the frame
		frame.getContentPane().add(viewer);
		frame.pack();
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		
		final byte[] imageData = ((DataBufferByte) image.getRaster()
				.getDataBuffer()).getData();

		final byte[] board = new byte[width * height];

		for (int i = 0; i < width * height; i++) {
			board[i] = 0;
		}
		
		for(int i=0;i<10;i++) {
			board[i+width*(height/2)+((width/2)-5)] = 1;
		}
		
		GameOfLife game = new GameOfLife(width, height, board);

		// Window listener to dispose Kernel resources on user exit.
		frame.addWindowListener(new WindowAdapter() {
			public void windowClosing(WindowEvent _windowEvent) {
				System.exit(0);
			}
		});

		
		
		long time = 0;
		long frames = 0;
		do {
			long t1 = System.currentTimeMillis();
			game.execute(width * height);

			for (int i = 0; i < width * height; i++) {
				board[i] /= 2;
				imageData[i] = (byte)(board[i]*255);
			}
			
			viewer.repaint();
			frames++;
			time += System.currentTimeMillis() - t1;
			
			if(time > 2000) {
				frame.setTitle("Game of Life - fps: "+(frames*1000.0)/(time));
				time = 0;
				frames = 0;
			}
		} while (true);

	}

}
