package aparapi.examples;
import com.amd.aparapi.Kernel;

public class GameOfLife extends Kernel {

	private final byte[] board;
	private final int width;
	private final int size;
	
	public GameOfLife(int width, int height, byte[] board) {
		this.width = width;
		this.size = width*height;
		this.board = board;
	}
	
	private byte getValue(int pos) {
		if(pos>=0) return (byte)(board[pos%size] & 1);
		else return (byte)(board[size-1 - (abs(pos)%size)] & 1);
	}
	
	@Override public void run() {
		int pos = getGlobalId();
		
		int sum = getValue(pos-1) + getValue(pos+1) +
		getValue(pos-width-1) + getValue(pos-width) + getValue(pos-width+1) +
		getValue(pos+width-1) + getValue(pos+width) + getValue(pos+width+1);
		
		if (sum==3 || sum==2) board[pos] += 2;
	}
	
}
