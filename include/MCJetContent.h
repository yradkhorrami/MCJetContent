#ifndef MCJetContent_h
#define MCJetContent_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <string>
class MCJetContent : public Processor
{

	public:

		virtual Processor*  newProcessor()
		{
			return new MCJetContent;
		}
		MCJetContent() ;
		virtual ~MCJetContent() = default;
		MCJetContent(const MCJetContent&) = delete;
		MCJetContent& operator=(const MCJetContent&) = delete;
		virtual void				init();
		virtual void				processRunHeader() ;
		virtual void				processEvent( EVENT::LCEvent *pLCEvent ) ;
		virtual void				check( LCEvent * evt ) ;
		virtual void				end() ;
    
	private:

		std::string				jetcollection{};
		std::string				MCPcollection{} ;
};

#endif  
