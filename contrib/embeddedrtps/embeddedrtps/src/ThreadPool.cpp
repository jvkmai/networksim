/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#include "rtps/ThreadPool.h"
#include <thread>
#include <iostream>

#include "lwip/tcpip.h"
#include "rtps/entities/Writer.h"
#include "rtps/utils/Log.h"
#include "rtps/utils/udpUtils.h"

using rtps::ThreadPool;

#if THREAD_POOL_VERBOSE && RTPS_GLOBAL_VERBOSE
#include "rtps/utils/printutils.h"
#define THREAD_POOL_LOG(...)                                                   \
  if (true) {                                                                  \
    printf("[ThreadPool] ");                                                   \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define THREAD_POOL_LOG(...) //
#endif

ThreadPool::ThreadPool(receiveJumppad_fp receiveCallback, void *callee)
    : m_receiveJumppad(receiveCallback), m_callee(callee) {

  if (!m_queueOutgoing.init() || !m_queueIncoming.init()) {
    return;
  }
  err_t inputErr = sys_sem_new(&m_readerNotificationSem, 0);
  err_t outputErr = sys_sem_new(&m_writerNotificationSem, 0);

  ns3::Simulator::Schedule(ns3::MilliSeconds(0), &ThreadPool::doWriterWork, this);
  ns3::Simulator::Schedule(ns3::MilliSeconds(0), &ThreadPool::doReaderWork, this);

  if (inputErr != ERR_OK || outputErr != ERR_OK) {
    THREAD_POOL_LOG("ThreadPool: Failed to create Semaphores.\n");
  }
}

ThreadPool::~ThreadPool() {
  if (m_running) {
    stopThreads();
    sys_msleep(500);
  }

  if (sys_sem_valid(&m_readerNotificationSem)) {
    sys_sem_free(&m_readerNotificationSem);
  }
  if (sys_sem_valid(&m_writerNotificationSem)) {
    sys_sem_free(&m_writerNotificationSem);
  }
}

void ThreadPool::doWriterWork() {
  Writer *workload;
  auto isWorkToDo = m_queueOutgoing.moveFirstInto(workload);
  if (!isWorkToDo) {
    return;
  }
  workload->progress();
}

void ThreadPool::doReaderWork() {
    PacketInfo packet;
    auto isWorkToDo = m_queueIncoming.moveFirstInto(packet);
    if (!isWorkToDo) {
      return;
    }
    m_receiveJumppad(m_callee, const_cast<const PacketInfo &>(packet));
}

bool ThreadPool::startThreads() {
  if (m_running) {
    return true;
  }
  if (!sys_sem_valid(&m_readerNotificationSem) ||
      !sys_sem_valid(&m_writerNotificationSem)) {
    return false;
  }
  m_running = true;
  for (auto &thread : m_writers) {
    // TODO ID, err check, waitOnStop
    //writerThreadFunction(this);
    // thread = sys_thread_new("WriterThread", writerThreadFunction, this,
    //                         Config::THREAD_POOL_WRITER_STACKSIZE,
    //                         Config::THREAD_POOL_WRITER_PRIO);
  }

  for (auto &thread : m_readers) {
    // TODO ID, err check, waitOnStop
    //readerThreadFunction(this);
    // thread = sys_thread_new("ReaderThread", readerThreadFunction, this,
    //                         Config::THREAD_POOL_READER_STACKSIZE,
    //                         Config::THREAD_POOL_READER_PRIO);
  }
  return true;
}

void ThreadPool::stopThreads() {
  m_running = false;
  // This should call all the semaphores for each thread once, so they don't
  // stuck before ended.
  for (auto &thread : m_writers) {
    (void)thread;
    sys_sem_signal(&m_writerNotificationSem);
    sys_msleep(10);
  }
  for (auto &thread : m_readers) {
    (void)thread;
    sys_sem_signal(&m_readerNotificationSem);
    sys_msleep(10);
  }
  // TODO make sure they have finished. Seems to be sufficient for tests.
  // Not sufficient if threads shall actually be stopped during runtime.
  sys_msleep(10);
}

void ThreadPool::clearQueues() {
  m_queueOutgoing.clear();
  m_queueIncoming.clear();
}

bool ThreadPool::addWorkload(Writer *workload) {
  bool res = m_queueOutgoing.moveElementIntoBuffer(std::move(workload));
  if (res) {
    ns3::Simulator::Schedule(ns3::NanoSeconds(100), &ThreadPool::doWriterWork, this);
  }
  return res;
}

bool ThreadPool::addNewPacket(PacketInfo &&packet) {
  bool res = m_queueIncoming.moveElementIntoBuffer(std::move(packet));
  if (res) {
    ns3::Simulator::Schedule(ns3::NanoSeconds(100), &ThreadPool::doReaderWork, this);
  }
  return res;
}

void ThreadPool::writerThreadFunction(void *arg) {
  auto pool = static_cast<ThreadPool *>(arg);
  if (pool == nullptr) {

    THREAD_POOL_LOG("nullptr passed to writer function\n");

    return;
  }

  pool->doWriterWork();
}

void ThreadPool::readCallback(void *args, Ip4Port_t destPort, pbuf *pbuf,
                              const ip_addr_t * /*addr*/, Ip4Port_t port) {
  auto &pool = *static_cast<ThreadPool *>(args);

  PacketInfo packet;
  // TODO This is a workaround for chained pbufs caused by hardware limitations,
  // not a general fix
  if (pbuf->next != nullptr) {
    struct pbuf *test = pbuf_alloc(PBUF_RAW, pbuf->tot_len, PBUF_POOL);
    pbuf_copy(test, pbuf);
    pbuf_free(pbuf);
    pbuf = test;
  }
  packet.destAddr = {0}; // not relevant
  packet.destPort = destPort;
  packet.srcPort = port;
  packet.buffer = PBufWrapper{pbuf};
  if (!pool.addNewPacket(std::move(packet))) {
    THREAD_POOL_LOG("ThreadPool: dropped packet\n");
  }
}

void ThreadPool::readerThreadFunction(void *arg) {
  auto pool = static_cast<ThreadPool *>(arg);
  if (pool == nullptr) {

    THREAD_POOL_LOG("nullptr passed to reader function\n");

    return;
  }
  pool->doReaderWork();
}

#undef THREAD_POOL_VERBOSE
