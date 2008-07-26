#define VCMI_DLL
#pragma warning(disable:4355)
#include "Connection.h"
#include <boost/asio.hpp>
#include <boost/thread.hpp>
using namespace boost;
using namespace boost::asio::ip;

#define LOG(a) \
	if(logging)\
		out << a
#if defined(__hppa__) || \
    defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
    (defined(__MIPS__) && defined(__MISPEB__)) || \
    defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
    defined(__sparc__)
#define BIG_ENDIAN
#else
#define LIL_ENDIAN
#endif

void CConnection::init()
{
#ifdef LIL_ENDIAN
	myEndianess = true;
#else
	myEndianess = false;
#endif
	connected = true;
	std::string pom;
	//we got connection
	(*this) << std::string("Aiya!\n") << name << myEndianess; //identify ourselves
	(*this) >> pom >> pom >> contactEndianess;
	out << "Established connection with "<<pom<<std::endl;
	wmx = new boost::mutex;
	rmx = new boost::mutex;
}

CConnection::CConnection(std::string host, std::string port, std::string Name, std::ostream & Out)
:io_service(new asio::io_service), name(Name), out(Out)//, send(this), rec(this)
{
    system::error_code error = asio::error::host_not_found;
	socket = new tcp::socket(*io_service);
    tcp::resolver resolver(*io_service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(tcp::resolver::query(host,port));
    socket->connect(*endpoint_iterator, error);
	if (error){ delete socket;	throw "Can't establish connection :("; }
	init();
}
CConnection::CConnection(
			boost::asio::basic_stream_socket<boost::asio::ip::tcp , boost::asio::stream_socket_service<boost::asio::ip::tcp>  > * Socket, 
			std::string Name, 
			std::ostream & Out	)
			:socket(Socket),io_service(&Socket->io_service()), out(Out), name(Name)//, send(this), rec(this)
{
	init();
}
CConnection::CConnection(boost::asio::basic_socket_acceptor<boost::asio::ip::tcp, boost::asio::socket_acceptor_service<boost::asio::ip::tcp> > * acceptor, boost::asio::io_service *Io_service, std::string Name, std::ostream & Out)
: out(Out), name(Name)//, send(this), rec(this)
{
    system::error_code error = asio::error::host_not_found;
	socket = new tcp::socket(*io_service);
	acceptor->accept(*socket,error);
	if (error){ delete socket;	throw "Can't establish connection :("; }
	init();
}
int CConnection::write(const void * data, unsigned size)
{
	LOG("Sending " << size << " byte(s) of data" <<std::endl);
	int ret;
	ret = asio::write(*socket,asio::const_buffers_1(asio::const_buffer(data,size)));
	return ret;
}
int CConnection::read(void * data, unsigned size)
{
	LOG("Receiving " << size << " byte(s) of data" <<std::endl);
	int ret = asio::read(*socket,asio::mutable_buffers_1(asio::mutable_buffer(data,size)));
	return ret;
}
CConnection::~CConnection(void)
{
	if(socket)
		socket->close();
	delete socket;
	delete io_service;
	delete wmx;
	delete rmx;
}
