import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
import org.apache.hadoop.filecache.DistributedCache;
import java.net.URI;
import java.io.File;
import org.apache.hadoop.mapreduce.JobContext;
import org.apache.hadoop.mapred.JobConf;
import org.apache.hadoop.mapreduce.Counters;
import org.apache.hadoop.mapreduce.Counter;
import org.apache.hadoop.mapreduce.CounterGroup;
import java.util.Iterator;

public class StaticHashing {


	static enum TotalSum { STARTS_WITH_DIGIT, STARTS_WITH_LETTER, ALL }


	static boolean useDynamicHashing = false;

	public static class TokenizerMapper 
		extends Mapper<Object, Text, Text, IntWritable>{
			static {
			}

			private final static IntWritable one = new IntWritable(1);
			private Text word = new Text();
			StaticHashing sa = new StaticHashing(); 

			public void map(Object key, Text value, Context context
				       ) throws IOException, InterruptedException {
				context.getCounter(TotalSum.STARTS_WITH_DIGIT).increment(1);
				Configuration job = ((JobContext)context).getConfiguration();
				StringTokenizer itr = new StringTokenizer(value.toString());
				while (itr.hasMoreTokens()) {
					word.set(itr.nextToken());
					if (useDynamicHashing){
						if (new Integer(word.toString()) < 500)
							word.set("1");
						else
							word.set("2");

					}
					context.write(word, one);
				}
			}
		}

	public static class IntSumReducer 
		extends Reducer<Text,IntWritable,Text,IntWritable> {
			private IntWritable result = new IntWritable();

			public void reduce(Text key, Iterable<IntWritable> values, 
					Context context
					) throws IOException, InterruptedException {
				int sum = 0;
				for (IntWritable val : values) {
					sum += val.get();
				}

			result.set(sum);
			context.write(key, result);
		}
}

public static void main(String[] args) throws Exception {
	Configuration conf = new Configuration();
	int simpleCount = 0;
	String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
	if (otherArgs.length != 2) {
		System.err.println("Usage: wordcount <in> <out>");
		System.exit(2);
	}
	double startTime =  System.currentTimeMillis();
	Job job = null;
	job = new Job(conf, "Fixed Iteration Experiment");
	FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]+simpleCount));
	job.setJarByClass(StaticHashing.class);
	job.setMapperClass(TokenizerMapper.class);
	job.setCombinerClass(IntSumReducer.class);
	job.setReducerClass(IntSumReducer.class);
	job.setOutputKeyClass(Text.class);
	job.setOutputValueClass(IntWritable.class);
	job.setNumReduceTasks(2);
	FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
	job.waitForCompletion(true);

	org.apache.hadoop.mapreduce.Counter c = job.getCounters().findCounter("StaticHashing$TotalSum","STARTS_WITH_DIGIT");
	System.out.println("Total time: " + (System.currentTimeMillis() - startTime) + "ms" + " my count: " + c.getValue());
}
}
