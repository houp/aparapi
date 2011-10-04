package aparapi.examples.gameoflife;
import com.amd.aparapi.Kernel;

public class GameOfLife extends Kernel {

	private int[] result;
	private int[] initial;
	private int width;
	private int height;
	private int size;
	public GameOfLife(int width, int height, int[] initial, int[] result) {
		this.width = width;
		this.height = height;
		this.initial = initial;
		this.size = width*height;
		this.result = result;
	}
	
	
	private int getValue(int pos) {
		if(pos>=0 && pos < size) return initial[pos];
		if(pos<0) return initial[size-1 - (abs(pos)%size)];
		return initial[pos%size];
	}
	
	@Override public void run() {
		int pos = getGlobalId();
		
		int sum = getValue(pos-1) + getValue(pos+1) +
		getValue(pos-width-1) + getValue(pos-width) + getValue(pos-width+1) +
		getValue(pos+width-1) + getValue(pos+width) + getValue(pos+width+1);
		
		if(sum!=3 && sum!=2) result[pos] = 0;
		else result[pos] = 1;
	}
	
}
