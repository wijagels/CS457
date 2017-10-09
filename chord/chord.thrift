typedef string UserID

exception SystemException {
  1: optional string message
}

struct RFileMetadata {
  1: optional string filename;
  2: optional i32 version;
  3: optional UserID owner;
  4: optional string contentHash;
}

struct RFile {
  1: optional RFileMetadata meta;
  2: optional string content;
}

struct NodeID {
  1: string id;
  2: string ip;
  3: i32 port;
}

service FileStore {
  void writeFile(1: RFile rFile)
    throws (1: SystemException systemException),

  RFile readFile(1: string filename, 2: UserID owner)
    throws (1: SystemException systemException),

  void setFingertable(1: list<NodeID> node_list),

  NodeID findSucc(1: string key)
    throws (1: SystemException systemException),

  NodeID findPred(1: string key)
    throws (1: SystemException systemException),

  NodeID getNodeSucc()
    throws (1: SystemException systemException),

  void setPredecessor(1: NodeID node),
}
