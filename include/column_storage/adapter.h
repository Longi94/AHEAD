#ifndef ADAPTER_H
#define ADAPTER_H

//#include ""
//#include <>

/**
* An adapter class (interface) used for IPC
* Existing adapters are:
* - SharedMemAdapter (using a shared memory)
* - QueueAdapter (using boost::message_queue)
*/
template<class T>
class Adapter {
public:
  virtual void init(bool write) = 0;
  virtual T read() = 0;
  virtual void write(T el) = 0;
  virtual void done() = 0;
protected:
  virtual ~Adapter() {/*std::cout << "~Adapter()!" << std::endl;*/}//destructor
};

#endif //ADAPTER_H
