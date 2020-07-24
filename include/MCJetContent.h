#ifndef MCJetContent_h
#define MCJetContent_h 1

#include "marlin/Processor.h"
#include "lcio.h"
#include <EVENT/MCParticle.h>
#include "IMPL/LCCollectionVec.h"
#include <IMPL/ReconstructedParticleImpl.h>
#include <string>

using namespace lcio ;
using namespace marlin ;

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
		virtual void				addMCPstoMCPartonShower( LCCollectionVec *m_col_outputPfo , EVENT::MCParticle *mcp ) ;
		virtual void				check( LCEvent * evt ) ;
		virtual void				end() ;
		void					Clear();

	private:

		std::string				m_MCPartonShower{};
		std::string				m_MCPcollection{} ;
		int					m_nRun;
		int					m_nEvt;
		int					m_nRunSum;
		int					m_nEvtSum;
		bool					m_includeNu;
		int					n_PartonShower;
};

#endif
