#include <iostream>
#include <boost/thread.hpp>

using namespace std;

void waitSec( int sec )
{
	boost::posix_time::seconds fooTime( sec );

	// io thread-safe?
	cout << "[wait] running" << endl;

	boost::this_thread::sleep( fooTime );

	cout << "[wait] finished" << endl;
};

void waitThread( int sec )
{
	boost::thread wait( waitSec, sec );
	cout << "[main] wait " << sec << " seconds..." << endl;
	wait.join();
}

class SimpleTimerEvent
{
public:
	void start( int msec )
	{
		m_interval = msec;
		m_thread = boost::thread( &SimpleTimerEvent::timerthread, this );
	}

	void stop()
	{
		m_thread.interrupt();
	}

protected:
	virtual void exec()=0;

private:
	void timerthread()
	{
		while( true )
		{
			boost::posix_time::millisec delay( m_interval );
			boost::this_thread::sleep( delay );
			exec();
		}
	}

	boost::thread m_thread;
	int m_interval;
};

class Bar : public SimpleTimerEvent
{
protected:
	void exec()
	{
		static long n=0;
		cout << "[Bar] exec() " << n++ << endl;
	}
};

int main()
{
	cout << "[main] hello boost threads!" << endl;

	Bar bar;
	cout << "[main] starting Bar SimpleTimerEvent" << endl;
	bar.start( 100 );

	waitThread( 1 );

	cout << "[main] stopping Bar SimpleTimerEvent" << endl;
	bar.stop();

	//waitThread( 1 );
	boost::this_thread::sleep( boost::posix_time::seconds(3) );

	cout << "[main] done" << endl;
	return 0;
}
