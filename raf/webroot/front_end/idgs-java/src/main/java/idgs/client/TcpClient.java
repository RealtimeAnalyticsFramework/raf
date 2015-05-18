
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.client;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicBoolean;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class TcpClient {
	
	private static Log log = LogFactory.getLog(TcpClient.class);
	
	/**
	 * judge whether connected
	 */
	private AtomicBoolean isConnected = new AtomicBoolean(false);
	
	/**
	 * server's socket address
	 */
	private SocketAddress servAddr;
	
	/**
	 * selector
	 */
  private Selector selector;
  /**
   * connected socket channel
   */
  private SocketChannel channel = null;
	
	/**
	 *  server's (ip + port)
	 */
	private String host;
	private int port;

	/**
	 *  client's id
	 */
	private int id;
	
	private long timeout = -1;
	
	/**
	 * send/recv message handler
	 */
	private SocketChannelHandler handler = new SocketChannelHandler();
	
	/**
	 * Construction
	 * @param host host's ip to be connected
	 * @param port host's port to be connected
	 * @throws IOException 
	 */
	TcpClient(String host, int port) throws IOException {
	  this.host = host;
	  this.port = port;
		initialize();
	}
	
	private void initialize() throws IOException {
		servAddr = new InetSocketAddress(host, port);
    // create selector
    try {
      selector = Selector.open();
    } catch (IOException e) {
      log.error(e.getMessage(), e);
      throw e;
    } 
	}
	
	public void setHandler(SocketChannelHandler handler) {
	  if(handler == null) {
	    throw new NullPointerException("SocketChannelHandler can not be null");
	  }
    this.handler = handler;
  }
	
	public void setTimeout(long timeout) {
	  this.timeout = timeout;
	}
	
	private synchronized void select() throws IOException {
	  if (timeout > 0) {
	    if (selector.select(timeout) == 0) {
	      throw new SocketTimeoutException();
	    }
	  } else {
	    selector.select();
	  }
	  
	  Iterator<SelectionKey> it = selector.selectedKeys().iterator();
	  while(it.hasNext()) {
      SelectionKey key = it.next();
      // handle connect
      if(key.isConnectable()) {
        processConnect(key);
      }
      // handle read
      else if(key.isReadable()) {
        processRead(key);
      }
      // handle write
      else if(key.isWritable()) {
        processWrite(key);
      }
      it.remove();
    }
	}
	
	/**
	 * 
	 * @param timeout
	 * @param retryTimes
	 * @throws IOException
	 */
	public boolean connect(final boolean enableTimeoutCheck, final int timeout, final int retryTimes) {
	  if(enableTimeoutCheck) {
	    if(retryTimes <= 0) { // up to re-try times
	      return isConnected();
	    }
  	  Timer timer = new Timer(); // after timeout seconds, run timer task, if not connected, re-try again
  	  timer.schedule(new TimerTask() {
  	    public void run() {
  	      if(!isConnected()) {
  	        int newRetryTimes = retryTimes - 1;
  	        connect(enableTimeoutCheck, timeout, newRetryTimes);
  	      }
  	    }
  	  }, timeout);
	  }
    try {
      SocketChannel channel = SocketChannel.open();
      channel.configureBlocking(false);
      channel.register(selector, SelectionKey.OP_CONNECT);
      channel.connect(servAddr);
      select();
    } catch (IOException e) {
      log.warn("try to connecte to server[" + servAddr.toString() + "] error, " + e.getMessage());
    }
    return isConnected();
	}
	
	public boolean isConnected() {
	  return isConnected.get();
	}
	
	private void processConnect(SelectionKey key) throws IOException {
	  if(!isConnected.get()) {
      if(key.isConnectable()) {
        channel = (SocketChannel) key.channel();
        if (channel.isConnectionPending()) {
          channel.finishConnect();
          channel.configureBlocking(false);
          isConnected.set(true);
          log.debug("connected to server: " + host);
          handler.onConnected(channel);
        }
      }
	  } else {
	    log.debug("already connected...");
	  }
	}
	
	private void processRead(SelectionKey key) throws IOException {
	  log.debug("-------------begin to process read-----------");
	  SocketChannel channel = (SocketChannel) key.channel();
	  channel.configureBlocking(false);
    handler.onRead(channel);
    log.debug("-------------end to process read-----------");
	}

	private void processWrite(SelectionKey key) throws IOException {
	  log.debug("-------------process write-----------");
	  SocketChannel channel = (SocketChannel) key.channel();
    handler.onWrite(channel);
	  log.debug("-------------end to process write-----------");
	}
	
	public void setId(int id) {
		this.id = id;
	}

	public int getId() {
		return this.id;
	}
	
	public void write() throws IOException {
	  channel.register(selector, SelectionKey.OP_WRITE);
	  select();
	}
	
	public void read() throws IOException {
	  channel.register(selector, SelectionKey.OP_READ);
	  select();
	}
	
	public void close() {
		if(channel == null) {
			return;
		}
		if(isConnected() && channel.isConnected()) {
			try {
				channel.close();
			} catch (IOException e) {
				log.error(e.getMessage());
			}
			log.debug("channel is closed");
		}
	}
	
	public String toString() {
		StringBuilder buf = new StringBuilder();
		buf.append("client id: ").append(id).append(",").append(" connected to server: ").append(host).append(":").append(port);
		return buf.toString();
	}
}
