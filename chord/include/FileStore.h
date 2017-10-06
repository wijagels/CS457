/**
 * Autogenerated by Thrift Compiler (0.10.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef FileStore_H
#define FileStore_H

#include <thrift/TDispatchProcessor.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>
#include "chord_types.h"



#ifdef _WIN32
  #pragma warning( push )
  #pragma warning (disable : 4250 ) //inheriting methods via dominance
#endif

class FileStoreIf {
 public:
  virtual ~FileStoreIf() {}
  virtual void writeFile(const RFile& rFile) = 0;
  virtual void readFile(RFile& _return, const std::string& filename, const UserID& owner) = 0;
  virtual void setFingertable(const std::vector<NodeID> & node_list) = 0;
  virtual void findSucc(NodeID& _return, const std::string& key) = 0;
  virtual void findPred(NodeID& _return, const std::string& key) = 0;
  virtual void getNodeSucc(NodeID& _return) = 0;
};

class FileStoreIfFactory {
 public:
  typedef FileStoreIf Handler;

  virtual ~FileStoreIfFactory() {}

  virtual FileStoreIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(FileStoreIf* /* handler */) = 0;
};

class FileStoreIfSingletonFactory : virtual public FileStoreIfFactory {
 public:
  FileStoreIfSingletonFactory(const boost::shared_ptr<FileStoreIf>& iface) : iface_(iface) {}
  virtual ~FileStoreIfSingletonFactory() {}

  virtual FileStoreIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(FileStoreIf* /* handler */) {}

 protected:
  boost::shared_ptr<FileStoreIf> iface_;
};

class FileStoreNull : virtual public FileStoreIf {
 public:
  virtual ~FileStoreNull() {}
  void writeFile(const RFile& /* rFile */) {
    return;
  }
  void readFile(RFile& /* _return */, const std::string& /* filename */, const UserID& /* owner */) {
    return;
  }
  void setFingertable(const std::vector<NodeID> & /* node_list */) {
    return;
  }
  void findSucc(NodeID& /* _return */, const std::string& /* key */) {
    return;
  }
  void findPred(NodeID& /* _return */, const std::string& /* key */) {
    return;
  }
  void getNodeSucc(NodeID& /* _return */) {
    return;
  }
};

typedef struct _FileStore_writeFile_args__isset {
  _FileStore_writeFile_args__isset() : rFile(false) {}
  bool rFile :1;
} _FileStore_writeFile_args__isset;

class FileStore_writeFile_args {
 public:

  FileStore_writeFile_args(const FileStore_writeFile_args&);
  FileStore_writeFile_args& operator=(const FileStore_writeFile_args&);
  FileStore_writeFile_args() {
  }

  virtual ~FileStore_writeFile_args() throw();
  RFile rFile;

  _FileStore_writeFile_args__isset __isset;

  void __set_rFile(const RFile& val);

  bool operator == (const FileStore_writeFile_args & rhs) const
  {
    if (!(rFile == rhs.rFile))
      return false;
    return true;
  }
  bool operator != (const FileStore_writeFile_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_writeFile_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_writeFile_pargs {
 public:


  virtual ~FileStore_writeFile_pargs() throw();
  const RFile* rFile;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_writeFile_result__isset {
  _FileStore_writeFile_result__isset() : systemException(false) {}
  bool systemException :1;
} _FileStore_writeFile_result__isset;

class FileStore_writeFile_result {
 public:

  FileStore_writeFile_result(const FileStore_writeFile_result&);
  FileStore_writeFile_result& operator=(const FileStore_writeFile_result&);
  FileStore_writeFile_result() {
  }

  virtual ~FileStore_writeFile_result() throw();
  SystemException systemException;

  _FileStore_writeFile_result__isset __isset;

  void __set_systemException(const SystemException& val);

  bool operator == (const FileStore_writeFile_result & rhs) const
  {
    if (!(systemException == rhs.systemException))
      return false;
    return true;
  }
  bool operator != (const FileStore_writeFile_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_writeFile_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_writeFile_presult__isset {
  _FileStore_writeFile_presult__isset() : systemException(false) {}
  bool systemException :1;
} _FileStore_writeFile_presult__isset;

class FileStore_writeFile_presult {
 public:


  virtual ~FileStore_writeFile_presult() throw();
  SystemException systemException;

  _FileStore_writeFile_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _FileStore_readFile_args__isset {
  _FileStore_readFile_args__isset() : filename(false), owner(false) {}
  bool filename :1;
  bool owner :1;
} _FileStore_readFile_args__isset;

class FileStore_readFile_args {
 public:

  FileStore_readFile_args(const FileStore_readFile_args&);
  FileStore_readFile_args& operator=(const FileStore_readFile_args&);
  FileStore_readFile_args() : filename(), owner() {
  }

  virtual ~FileStore_readFile_args() throw();
  std::string filename;
  UserID owner;

  _FileStore_readFile_args__isset __isset;

  void __set_filename(const std::string& val);

  void __set_owner(const UserID& val);

  bool operator == (const FileStore_readFile_args & rhs) const
  {
    if (!(filename == rhs.filename))
      return false;
    if (!(owner == rhs.owner))
      return false;
    return true;
  }
  bool operator != (const FileStore_readFile_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_readFile_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_readFile_pargs {
 public:


  virtual ~FileStore_readFile_pargs() throw();
  const std::string* filename;
  const UserID* owner;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_readFile_result__isset {
  _FileStore_readFile_result__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_readFile_result__isset;

class FileStore_readFile_result {
 public:

  FileStore_readFile_result(const FileStore_readFile_result&);
  FileStore_readFile_result& operator=(const FileStore_readFile_result&);
  FileStore_readFile_result() {
  }

  virtual ~FileStore_readFile_result() throw();
  RFile success;
  SystemException systemException;

  _FileStore_readFile_result__isset __isset;

  void __set_success(const RFile& val);

  void __set_systemException(const SystemException& val);

  bool operator == (const FileStore_readFile_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(systemException == rhs.systemException))
      return false;
    return true;
  }
  bool operator != (const FileStore_readFile_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_readFile_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_readFile_presult__isset {
  _FileStore_readFile_presult__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_readFile_presult__isset;

class FileStore_readFile_presult {
 public:


  virtual ~FileStore_readFile_presult() throw();
  RFile* success;
  SystemException systemException;

  _FileStore_readFile_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _FileStore_setFingertable_args__isset {
  _FileStore_setFingertable_args__isset() : node_list(false) {}
  bool node_list :1;
} _FileStore_setFingertable_args__isset;

class FileStore_setFingertable_args {
 public:

  FileStore_setFingertable_args(const FileStore_setFingertable_args&);
  FileStore_setFingertable_args& operator=(const FileStore_setFingertable_args&);
  FileStore_setFingertable_args() {
  }

  virtual ~FileStore_setFingertable_args() throw();
  std::vector<NodeID>  node_list;

  _FileStore_setFingertable_args__isset __isset;

  void __set_node_list(const std::vector<NodeID> & val);

  bool operator == (const FileStore_setFingertable_args & rhs) const
  {
    if (!(node_list == rhs.node_list))
      return false;
    return true;
  }
  bool operator != (const FileStore_setFingertable_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_setFingertable_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_setFingertable_pargs {
 public:


  virtual ~FileStore_setFingertable_pargs() throw();
  const std::vector<NodeID> * node_list;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_setFingertable_result {
 public:

  FileStore_setFingertable_result(const FileStore_setFingertable_result&);
  FileStore_setFingertable_result& operator=(const FileStore_setFingertable_result&);
  FileStore_setFingertable_result() {
  }

  virtual ~FileStore_setFingertable_result() throw();

  bool operator == (const FileStore_setFingertable_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const FileStore_setFingertable_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_setFingertable_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_setFingertable_presult {
 public:


  virtual ~FileStore_setFingertable_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _FileStore_findSucc_args__isset {
  _FileStore_findSucc_args__isset() : key(false) {}
  bool key :1;
} _FileStore_findSucc_args__isset;

class FileStore_findSucc_args {
 public:

  FileStore_findSucc_args(const FileStore_findSucc_args&);
  FileStore_findSucc_args& operator=(const FileStore_findSucc_args&);
  FileStore_findSucc_args() : key() {
  }

  virtual ~FileStore_findSucc_args() throw();
  std::string key;

  _FileStore_findSucc_args__isset __isset;

  void __set_key(const std::string& val);

  bool operator == (const FileStore_findSucc_args & rhs) const
  {
    if (!(key == rhs.key))
      return false;
    return true;
  }
  bool operator != (const FileStore_findSucc_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_findSucc_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_findSucc_pargs {
 public:


  virtual ~FileStore_findSucc_pargs() throw();
  const std::string* key;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_findSucc_result__isset {
  _FileStore_findSucc_result__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_findSucc_result__isset;

class FileStore_findSucc_result {
 public:

  FileStore_findSucc_result(const FileStore_findSucc_result&);
  FileStore_findSucc_result& operator=(const FileStore_findSucc_result&);
  FileStore_findSucc_result() {
  }

  virtual ~FileStore_findSucc_result() throw();
  NodeID success;
  SystemException systemException;

  _FileStore_findSucc_result__isset __isset;

  void __set_success(const NodeID& val);

  void __set_systemException(const SystemException& val);

  bool operator == (const FileStore_findSucc_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(systemException == rhs.systemException))
      return false;
    return true;
  }
  bool operator != (const FileStore_findSucc_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_findSucc_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_findSucc_presult__isset {
  _FileStore_findSucc_presult__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_findSucc_presult__isset;

class FileStore_findSucc_presult {
 public:


  virtual ~FileStore_findSucc_presult() throw();
  NodeID* success;
  SystemException systemException;

  _FileStore_findSucc_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _FileStore_findPred_args__isset {
  _FileStore_findPred_args__isset() : key(false) {}
  bool key :1;
} _FileStore_findPred_args__isset;

class FileStore_findPred_args {
 public:

  FileStore_findPred_args(const FileStore_findPred_args&);
  FileStore_findPred_args& operator=(const FileStore_findPred_args&);
  FileStore_findPred_args() : key() {
  }

  virtual ~FileStore_findPred_args() throw();
  std::string key;

  _FileStore_findPred_args__isset __isset;

  void __set_key(const std::string& val);

  bool operator == (const FileStore_findPred_args & rhs) const
  {
    if (!(key == rhs.key))
      return false;
    return true;
  }
  bool operator != (const FileStore_findPred_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_findPred_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_findPred_pargs {
 public:


  virtual ~FileStore_findPred_pargs() throw();
  const std::string* key;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_findPred_result__isset {
  _FileStore_findPred_result__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_findPred_result__isset;

class FileStore_findPred_result {
 public:

  FileStore_findPred_result(const FileStore_findPred_result&);
  FileStore_findPred_result& operator=(const FileStore_findPred_result&);
  FileStore_findPred_result() {
  }

  virtual ~FileStore_findPred_result() throw();
  NodeID success;
  SystemException systemException;

  _FileStore_findPred_result__isset __isset;

  void __set_success(const NodeID& val);

  void __set_systemException(const SystemException& val);

  bool operator == (const FileStore_findPred_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(systemException == rhs.systemException))
      return false;
    return true;
  }
  bool operator != (const FileStore_findPred_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_findPred_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_findPred_presult__isset {
  _FileStore_findPred_presult__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_findPred_presult__isset;

class FileStore_findPred_presult {
 public:


  virtual ~FileStore_findPred_presult() throw();
  NodeID* success;
  SystemException systemException;

  _FileStore_findPred_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class FileStore_getNodeSucc_args {
 public:

  FileStore_getNodeSucc_args(const FileStore_getNodeSucc_args&);
  FileStore_getNodeSucc_args& operator=(const FileStore_getNodeSucc_args&);
  FileStore_getNodeSucc_args() {
  }

  virtual ~FileStore_getNodeSucc_args() throw();

  bool operator == (const FileStore_getNodeSucc_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const FileStore_getNodeSucc_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_getNodeSucc_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class FileStore_getNodeSucc_pargs {
 public:


  virtual ~FileStore_getNodeSucc_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_getNodeSucc_result__isset {
  _FileStore_getNodeSucc_result__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_getNodeSucc_result__isset;

class FileStore_getNodeSucc_result {
 public:

  FileStore_getNodeSucc_result(const FileStore_getNodeSucc_result&);
  FileStore_getNodeSucc_result& operator=(const FileStore_getNodeSucc_result&);
  FileStore_getNodeSucc_result() {
  }

  virtual ~FileStore_getNodeSucc_result() throw();
  NodeID success;
  SystemException systemException;

  _FileStore_getNodeSucc_result__isset __isset;

  void __set_success(const NodeID& val);

  void __set_systemException(const SystemException& val);

  bool operator == (const FileStore_getNodeSucc_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(systemException == rhs.systemException))
      return false;
    return true;
  }
  bool operator != (const FileStore_getNodeSucc_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const FileStore_getNodeSucc_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _FileStore_getNodeSucc_presult__isset {
  _FileStore_getNodeSucc_presult__isset() : success(false), systemException(false) {}
  bool success :1;
  bool systemException :1;
} _FileStore_getNodeSucc_presult__isset;

class FileStore_getNodeSucc_presult {
 public:


  virtual ~FileStore_getNodeSucc_presult() throw();
  NodeID* success;
  SystemException systemException;

  _FileStore_getNodeSucc_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class FileStoreClient : virtual public FileStoreIf {
 public:
  FileStoreClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  FileStoreClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void writeFile(const RFile& rFile);
  void send_writeFile(const RFile& rFile);
  void recv_writeFile();
  void readFile(RFile& _return, const std::string& filename, const UserID& owner);
  void send_readFile(const std::string& filename, const UserID& owner);
  void recv_readFile(RFile& _return);
  void setFingertable(const std::vector<NodeID> & node_list);
  void send_setFingertable(const std::vector<NodeID> & node_list);
  void recv_setFingertable();
  void findSucc(NodeID& _return, const std::string& key);
  void send_findSucc(const std::string& key);
  void recv_findSucc(NodeID& _return);
  void findPred(NodeID& _return, const std::string& key);
  void send_findPred(const std::string& key);
  void recv_findPred(NodeID& _return);
  void getNodeSucc(NodeID& _return);
  void send_getNodeSucc();
  void recv_getNodeSucc(NodeID& _return);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class FileStoreProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<FileStoreIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (FileStoreProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_writeFile(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_readFile(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_setFingertable(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_findSucc(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_findPred(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_getNodeSucc(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  FileStoreProcessor(boost::shared_ptr<FileStoreIf> iface) :
    iface_(iface) {
    processMap_["writeFile"] = &FileStoreProcessor::process_writeFile;
    processMap_["readFile"] = &FileStoreProcessor::process_readFile;
    processMap_["setFingertable"] = &FileStoreProcessor::process_setFingertable;
    processMap_["findSucc"] = &FileStoreProcessor::process_findSucc;
    processMap_["findPred"] = &FileStoreProcessor::process_findPred;
    processMap_["getNodeSucc"] = &FileStoreProcessor::process_getNodeSucc;
  }

  virtual ~FileStoreProcessor() {}
};

class FileStoreProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  FileStoreProcessorFactory(const ::boost::shared_ptr< FileStoreIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< FileStoreIfFactory > handlerFactory_;
};

class FileStoreMultiface : virtual public FileStoreIf {
 public:
  FileStoreMultiface(std::vector<boost::shared_ptr<FileStoreIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~FileStoreMultiface() {}
 protected:
  std::vector<boost::shared_ptr<FileStoreIf> > ifaces_;
  FileStoreMultiface() {}
  void add(boost::shared_ptr<FileStoreIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void writeFile(const RFile& rFile) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->writeFile(rFile);
    }
    ifaces_[i]->writeFile(rFile);
  }

  void readFile(RFile& _return, const std::string& filename, const UserID& owner) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->readFile(_return, filename, owner);
    }
    ifaces_[i]->readFile(_return, filename, owner);
    return;
  }

  void setFingertable(const std::vector<NodeID> & node_list) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->setFingertable(node_list);
    }
    ifaces_[i]->setFingertable(node_list);
  }

  void findSucc(NodeID& _return, const std::string& key) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->findSucc(_return, key);
    }
    ifaces_[i]->findSucc(_return, key);
    return;
  }

  void findPred(NodeID& _return, const std::string& key) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->findPred(_return, key);
    }
    ifaces_[i]->findPred(_return, key);
    return;
  }

  void getNodeSucc(NodeID& _return) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->getNodeSucc(_return);
    }
    ifaces_[i]->getNodeSucc(_return);
    return;
  }

};

// The 'concurrent' client is a thread safe client that correctly handles
// out of order responses.  It is slower than the regular client, so should
// only be used when you need to share a connection among multiple threads
class FileStoreConcurrentClient : virtual public FileStoreIf {
 public:
  FileStoreConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  FileStoreConcurrentClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void writeFile(const RFile& rFile);
  int32_t send_writeFile(const RFile& rFile);
  void recv_writeFile(const int32_t seqid);
  void readFile(RFile& _return, const std::string& filename, const UserID& owner);
  int32_t send_readFile(const std::string& filename, const UserID& owner);
  void recv_readFile(RFile& _return, const int32_t seqid);
  void setFingertable(const std::vector<NodeID> & node_list);
  int32_t send_setFingertable(const std::vector<NodeID> & node_list);
  void recv_setFingertable(const int32_t seqid);
  void findSucc(NodeID& _return, const std::string& key);
  int32_t send_findSucc(const std::string& key);
  void recv_findSucc(NodeID& _return, const int32_t seqid);
  void findPred(NodeID& _return, const std::string& key);
  int32_t send_findPred(const std::string& key);
  void recv_findPred(NodeID& _return, const int32_t seqid);
  void getNodeSucc(NodeID& _return);
  int32_t send_getNodeSucc();
  void recv_getNodeSucc(NodeID& _return, const int32_t seqid);
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
  ::apache::thrift::async::TConcurrentClientSyncInfo sync_;
};

#ifdef _WIN32
  #pragma warning( pop )
#endif



#endif
