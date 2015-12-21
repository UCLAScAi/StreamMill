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

public class Apriori {


	static enum TotalSum { STARTS_WITH_DIGIT, STARTS_WITH_LETTER, ALL }

	static boolean onlyLocalIteration = true;
	static int numOfIterations = 2;


	public static class TokenizerMapper 
		extends Mapper<Object, Text, Text, IntWritable>{
			static {
			}

			private final static IntWritable one = new IntWritable(1);
			private Text word = new Text();
			Apriori sa = new Apriori(); 

			public void map(Object key, Text value, Context context
				       ) throws IOException, InterruptedException {
				context.getCounter(TotalSum.STARTS_WITH_DIGIT).increment(1);
				Configuration job = ((JobContext)context).getConfiguration();
				StringTokenizer itr = new StringTokenizer(value.toString());
				while (itr.hasMoreTokens()) {
					itr.nextToken();
					word.set("Sum");
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
				int localIts = 0;
				int sum = 0;
				while (localIts < numOfIterations) {
					try {
						Thread.currentThread().sleep(1000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					sum = 0;
					for (IntWritable val : values) {
						sum += val.get();
					}
					if (!onlyLocalIteration) 
						break;
					localIts++;
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
		while (simpleCount < numOfIterations) {
			job = new Job(conf, "Fixed Iteration Experiment");
			FileOutputFormat.setOutputPath(job, new Path(otherArgs[1]+simpleCount));
			job.setJarByClass(Apriori.class);
			job.setMapperClass(TokenizerMapper.class);
			job.setCombinerClass(IntSumReducer.class);
			job.setReducerClass(IntSumReducer.class);
			job.setOutputKeyClass(Text.class);
			job.setOutputValueClass(IntWritable.class);
			FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
			job.waitForCompletion(true);
			simpleCount++;
			if (onlyLocalIteration)
				break;
		}

		org.apache.hadoop.mapreduce.Counter c = job.getCounters().findCounter("Apriori$TotalSum","STARTS_WITH_DIGIT");
		if (onlyLocalIteration)
			System.out.print("Hybrid Iteration: " );
		System.out.println("Total time: " + (System.currentTimeMillis() - startTime) + "ms" + " my count: " + c.getValue());
	}
}
