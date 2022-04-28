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
		virtual void				finIsolatedLeptonFSR( EVENT::LCEvent *pLCEvent , EVENT::MCParticle *mcp, int leptonPDG , std::vector<int> &isolatedLeptonIndex , std::vector<int> &isolatedPhotonIndex ) ;
		virtual void				check( LCEvent * evt ) ;
		virtual void				end() ;
		void					Clear();

	private:

		std::string				m_MCJetContent_wInvisibles{};
		std::string				m_MCJetContent_woInvisibles{};
		std::string				m_MCISR{};
		std::string				m_MCFSR{};
		std::string				m_MCPcollection{} ;
		std::string				m_MCIsolatedLeptons{};
		int					m_nRun;
		int					m_nEvt;
		int					m_nRunSum;
		int					m_nEvtSum;
		float					m_minTheta;
		int					n_PartonShower;
		bool					m_includeOverlay{};
};

#endif
