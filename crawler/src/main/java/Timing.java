package crawltwitter;

public class Timing implements AutoCloseable {
	StackTraceElement _ste;
	long _start;

	public Timing()
	{
		_ste = Thread.currentThread().getStackTrace()[2];
		_start = System.nanoTime();
	}

	@Override
	public void close()
	{
		double duration = (System.nanoTime() - _start) / 1000000000.0;

		StringBuilder sb = new StringBuilder();
		sb.append(Thread.currentThread().getId());
		sb.append(" ");
		//sb.append(_ste.getClassName());
		//sb.append(".");
		sb.append(_ste.getMethodName());
		sb.append("()");
		//sb.append(" [");
		//sb.append(_ste.getFileName());
		//sb.append(":");
		//sb.append(_ste.getLineNumber());
		//sb.append("]");

		System.err.printf("%s %f sec\n", sb.toString(), duration);
	}
}
