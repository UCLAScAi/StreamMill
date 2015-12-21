import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

public class AutoTest {
	public String sendToServer(String ip, int port, String command) {
		System.out.println(Thread.currentThread() + " cmd " + command);
		StringBuffer buf = new StringBuffer();
		Socket client = null;
		DataInputStream is = null;
		OutputStream os = null;
		try {
			client = new Socket(ip, port);
			is = new DataInputStream(client.getInputStream());
			os = client.getOutputStream();
			os.write(command.getBytes());
			String responseLine = null;
			while ((responseLine = is.readLine()) != null) {
				buf.append(responseLine);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (os != null) {
				try {
					os.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (is != null) {
				try {
					is.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (client != null) {
				try {
					client.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return buf.toString();
	}

	public String sendToServerWithData(String ip, int port, String command,
			String data) {
		System.out.println(Thread.currentThread() + " cmd " + command + " data"
				+ data);
		StringBuffer buf = new StringBuffer();
		Socket client = null;
		DataInputStream is = null;
		OutputStream os = null;
		try {
			client = new Socket(ip, port);
			is = new DataInputStream(client.getInputStream());
			os = client.getOutputStream();
			os.write((command + "\n").getBytes());
			os.write(data.getBytes());
			String responseLine = null;
			while ((responseLine = is.readLine()) != null) {
				buf.append(responseLine);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (os != null) {
				try {
					os.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (is != null) {
				try {
					is.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (client != null) {
				try {
					client.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return buf.toString();
	}

	public void startListener(int monitoringPort, AtomicBoolean finished) {
		try {
			ServerSocket serverSocket = new ServerSocket(monitoringPort);
			serverSocket.setSoTimeout(10000);
			Socket clientSocket = null;
			DataInputStream in = null;
			String responseLine = null;
			while (true) {
				try {
					System.out.println(Thread.currentThread() + " waiting...");
					try {
						clientSocket = serverSocket.accept();
					} catch (Exception e) {
					}
					in = new DataInputStream(clientSocket.getInputStream());
					while ((responseLine = in.readLine()) != null) {
						System.out.println(Thread.currentThread() + " "
								+ responseLine);
					}
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					if (in != null) {
						try {
							in.close();
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
					if (clientSocket != null) {
						try {
							clientSocket.close();
						} catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
				if (finished.get()) {
					break;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void sendToServer(String ip, int port, String username,
			String filename) {
		System.out.println(Thread.currentThread() + " Sending data to server");
		BufferedReader br = null;
		Socket client = null;
		OutputStream os = null;
		try {
			br = new BufferedReader(new FileReader(filename));
			client = new Socket(ip, port);
			System.out
					.println(Thread.currentThread() + " " + client.toString());
			os = client.getOutputStream();
			String line = null, command = null;
			while ((line = br.readLine()) != null) {
				command = username + "__traffic," + line;
				System.out.println(Thread.currentThread() + " Sending:"
						+ command);
				os.write(command.getBytes());
				os.flush();
				try {
					TimeUnit.MILLISECONDS.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			if (br != null) {
				try {
					br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (os != null) {
				try {
					os.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (client != null) {
				try {
					client.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}

	public String loadFile(String filename) {
		StringBuilder bf = new StringBuilder();
		try {
			BufferedReader br = new BufferedReader(new FileReader(filename));
			String line = null;
			while ((line = br.readLine()) != null)
				bf.append(line + "\n");
			br.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return bf.toString();
	}

	public void run(final String ip, final int port, final String username,
			final int streamPort, final int monitoringPort) {
		String ret = null, streamModule = null, queryModule = null, queryId = null;
		int pos = 0, posStart = 0, posEnd = 0;
		ret = sendToServer(ip, port, "AddNewUser " + username + " " + username
				+ " " + username + "@cs.ucla.edu 123456");
		System.out.println(Thread.currentThread() + " " + ret);
		ret = sendToServerWithData(ip, port, "AddStreams " + username
				+ " s1.dcl",
				"STREAM traffic(station_id INT, speed INT, time TIMESTAMP) SOURCE 'port"
						+ streamPort + "';");
		System.out.println(Thread.currentThread() + " " + ret);
		ret = sendToServer(ip, port, "AddAggregate " + username + " "
				+ loadFile("sum.aggr"));
		System.out.println(Thread.currentThread() + " " + ret);
		ret = sendToServerWithData(ip, port, "AddQueries " + username
				+ " q1.cq",
				"SELECT sum_aggr(speed) over (RoWS 3 PRECEDING) FROM traffic;");
		System.out.println(Thread.currentThread() + " " + ret);
		ret = sendToServer(ip, port, "ViewAllStreams " + username);
		System.out.println(Thread.currentThread() + " " + ret);
		pos = ret.indexOf("s1.dcl");
		if (pos >= 0) {
			posStart = pos + "s1.dcl".length() + 1;
			posEnd = ret.indexOf(' ', posStart);
			streamModule = ret.substring(posStart, posEnd).trim();
			System.out.println(Thread.currentThread() + " " + streamModule);
		}
		ret = sendToServer(ip, port, "ViewAllQueries " + username);
		System.out.println(Thread.currentThread() + " " + ret);
		pos = ret.indexOf("q1.cq");
		if (pos >= 0) {
			posStart = pos + "q1.cq".length() + 1;
			posEnd = ret.indexOf(' ', posStart);
			queryModule = ret.substring(posStart, posEnd).trim();
			System.out.println(Thread.currentThread() + " " + queryModule);
			posStart = posEnd + 1;
			posEnd = ret.indexOf(' ', posStart);
			queryId = ret.substring(posStart, posEnd).trim();
			if (queryId.startsWith("no")) {
				queryId = queryId.substring(2);
			} else if (queryId.startsWith("yes")) {
				queryId = queryId.substring(3);
			}
			System.out.println(Thread.currentThread() + " " + queryId);
		}
		if (streamModule != null && queryModule != null && queryId != null) {
			ret = sendToServer(ip, port, "ActivateStreamModule " + username
					+ " " + streamModule);
			System.out.println(Thread.currentThread() + " " + ret);
			ret = sendToServer(ip, port, "ActivateQueryModule " + username
					+ " " + queryModule);
			System.out.println(Thread.currentThread() + " " + ret);
			ret = sendToServer(ip, port, "MonitorBuffer " + username
					+ " stdout_" + queryId + " o " + monitoringPort);
			System.out.println(Thread.currentThread() + " " + ret);
			final AutoTest selfPointer = this;
			final AtomicBoolean finished1 = new AtomicBoolean(false), finished2 = new AtomicBoolean(
					false);
			Thread dataThread = new Thread() {
				public void run() {
					try {
						TimeUnit.MILLISECONDS.sleep(1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					// selfPointer.sendToServer(ip, streamPort, username,
					// "update.txt");
					try {
						Process p = Runtime.getRuntime().exec(
								"./ss.pl " + username + "__traffic " + ip + " "
										+ streamPort + " 1 1");
						p.waitFor();
						BufferedReader reader = new BufferedReader(
								new InputStreamReader(p.getInputStream()));
						String line = null;
						while ((line = reader.readLine()) != null) {
							System.out.println(Thread.currentThread() + " "
									+ line);
						}
						reader.close();
					} catch (Exception e) {
						e.printStackTrace();
					}

					finished1.set(true);
				}
			}, monitorThread = new Thread() {
				public void run() {
					selfPointer.startListener(monitoringPort, finished1);
					finished2.set(true);
				}
			};
			monitorThread.start();
			dataThread.start();
			while (!finished2.get()) {
				try {
					TimeUnit.MILLISECONDS.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			ret = sendToServer(ip, port, "DeactivateStreamModule " + username
					+ " " + streamModule);
			System.out.println(Thread.currentThread() + " " + ret);
			ret = sendToServer(ip, port, "DeactivateQueryModule " + username
					+ " " + queryModule);
			System.out.println(Thread.currentThread() + " " + ret);
			ret = sendToServer(ip, port, "UnMonitorBuffer " + username
					+ " [q1.cq]-stdout_" + queryId + " o " + monitoringPort);
			System.out.println(Thread.currentThread() + " " + ret);
			ret = sendToServer(ip, port, "UnMonitorAllOfIP " + username);
			System.out.println(Thread.currentThread() + " " + ret);
		}
		if (queryModule != null) {
			ret = sendToServer(ip, port, "DeleteQueryModule " + username + " "
					+ queryModule);
			System.out.println(Thread.currentThread() + " " + ret);
		}
		ret = sendToServer(ip, port, "DeleteAggregate " + username + " "
				+ username + "__sum_aggr_window");
		System.out.println(Thread.currentThread() + " " + ret);
		if (streamModule != null) {
			ret = sendToServer(ip, port, "DeleteDeclareModule " + username
					+ " " + streamModule);
			System.out.println(Thread.currentThread() + " " + ret);
		}
	}

	public static void main(String[] args) {
		final AutoTest t = new AutoTest();
		final String ip = "localhost", username = "test";
		final int port = 5431, streamPort = 4439, monitoringPort = 8080;
		class TestThread extends Thread {
			int id = 0;

			public TestThread(int id) {
				super();
				this.id = id;
			}

			public void run() {
				t.run(ip, port, username + String.valueOf(id), streamPort + id,
						monitoringPort + id);
			}
		}
		int numOfThreads = 10;
		Thread[] threads = new Thread[numOfThreads];
		for (int i = 0; i < numOfThreads; ++i) {
			threads[i] = new TestThread(i);
			threads[i].start();
		}
		for (int i = 0; i < numOfThreads; ++i) {
			try {
				threads[i].join();
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}
	}
}