#include "MCJetContent.h"
#include <iostream>
#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/ReconstructedParticle.h>
#include "IMPL/ReconstructedParticleImpl.h"
#include "IMPL/LCCollectionVec.h"
#include <EVENT/LCCollection.h>

using namespace lcio ;
using namespace marlin ;
using namespace std ;

MCJetContent aMCJetContent;

MCJetContent::MCJetContent() :

Processor("MCJetContent"),
m_nRun(0),
m_nEvt(0),
m_nRunSum(0),
m_nEvtSum(0),
m_includeNu(true),
n_PartonShower(0)
{
	_description = "MCJetContent creates a new collection of ReconstructedParticle type and collects parton shower to form MCJet with JetFinder/JetClustering processor";

	registerInputCollection(	LCIO::MCPARTICLE,
					"MCParticleCollection" ,
					"Name of the MCParticle collection"  ,
					m_MCPcollection,
					std::string("MCParticle")
				);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
					"PartonShowerCollection",
					"Name of output Parton Shower collection",
					m_MCPartonShower,
					std::string("MCPartonShower")
				);
	registerProcessorParameter(	"IncludeNuinJet",
					"Include Nu in Jet contents",
					m_includeNu,
					bool(true)
					);
}

void MCJetContent::init()
{
	streamlog_out(DEBUG) << "   init called  " << std::endl;
	printParameters();
	m_nRun = 0 ;
	m_nEvt = 0 ;
	m_nRunSum = 0;
	m_nEvtSum = 0;
	n_PartonShower = 0;
	this->Clear();
}

void MCJetContent::processRunHeader()
{
	m_nRun = 0;
	m_nEvt = 0;
	++m_nRunSum;
}
void MCJetContent::Clear()
{
	n_PartonShower = -2;
}

void MCJetContent::check(EVENT::LCEvent *pLCEvent)
{
	LCCollection *outputPfoCollection{};
	try
	{
		outputPfoCollection = pLCEvent->getCollection(m_MCPartonShower);
		int n_inputHadrons = n_PartonShower;
		int n_outputPFOs = outputPfoCollection->getNumberOfElements();
		streamlog_out(MESSAGE) << " CHECK : processed events: " << m_nEvtSum << " (Number of mcHadrons: " << n_inputHadrons << " , Number of outputHadrons: " << n_outputPFOs <<")" << std::endl;
	}
	catch(DataNotAvailableException &e)
        {
		streamlog_out(MESSAGE) << "Input/Output collection not found in event " << m_nEvt << std::endl;
        }
}

void MCJetContent::processEvent( EVENT::LCEvent *pLCEvent )
{

	m_nRun = pLCEvent->getRunNumber();
	m_nEvt = pLCEvent->getEventNumber();
	++m_nEvtSum;

	LCCollection *MCParticleCollection{};
	this->Clear();


        try
        {
		MCParticleCollection = pLCEvent->getCollection(m_MCPcollection);
		LCCollectionVec *m_col_outputPfo = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		int n_MCP = MCParticleCollection->getNumberOfElements();

		for (int i_MCP = 0 ; i_MCP < n_MCP ; ++i_MCP)
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(i_MCP));
			if ( mcp->getPDG() == 92 )
			{
				this->addMCPstoMCPartonShower( m_col_outputPfo , mcp );
//				int n_hadrons = (mcp->getDaughters()).size();
//				for (int i_hadron = 0 ; i_hadron < n_hadrons ; ++i_hadron)
//				{
//					ReconstructedParticleImpl* mcHadron = new ReconstructedParticleImpl;
//					if ( mcHadron )
//					mcHadron->setType((mcp->getDaughters()[i_hadron])->getPDG());
//					mcHadron->setMomentum((mcp->getDaughters()[i_hadron])->getMomentum());
//					mcHadron->setEnergy((mcp->getDaughters()[i_hadron])->getEnergy());
//					mcHadron->setMass((mcp->getDaughters()[i_hadron])->getMass());
//					mcHadron->setCharge((mcp->getDaughters()[i_hadron])->getCharge());
//					mcHadron->setStartVertex((mcp->getDaughters()[i_hadron])->getVertex());
//					m_col_outputPfo->addElement( mcHadron );
//				}
			}
			if ( mcp->getGeneratorStatus() == 1 )
			{
				if ( abs(mcp->getPDG()) == 12 || abs(mcp->getPDG()) == 14 || abs(mcp->getPDG()) == 16 )
				{
					if ( m_includeNu ) n_PartonShower += 1;
				}
				else
				{
					n_PartonShower += 1;
				}
			}
		}
		pLCEvent->addCollection( m_col_outputPfo , m_MCPartonShower );
	}

        catch(DataNotAvailableException &e)
        {
		streamlog_out(MESSAGE) << "Input collection not found in event " << m_nEvt << std::endl;
        }

}

void MCJetContent::addMCPstoMCPartonShower( LCCollectionVec *m_col_outputPfo , EVENT::MCParticle *mcp )
{
		if ( mcp->getGeneratorStatus() == 1 )
		{
			if ( abs(mcp->getPDG()) == 12 || abs(mcp->getPDG()) == 14 || abs(mcp->getPDG()) == 16 )
			{
				if ( m_includeNu )
				{
					ReconstructedParticleImpl* mcHadron = new ReconstructedParticleImpl;
					mcHadron->setType(mcp->getPDG());
					mcHadron->setMomentum(mcp->getMomentum());
					mcHadron->setEnergy(mcp->getEnergy());
					mcHadron->setMass(mcp->getMass());
					mcHadron->setCharge(mcp->getCharge());
					m_col_outputPfo->addElement( mcHadron );
				}
			}
			else
			{
				ReconstructedParticleImpl* mcHadron = new ReconstructedParticleImpl;
				mcHadron->setType(mcp->getPDG());
				mcHadron->setMomentum(mcp->getMomentum());
				mcHadron->setEnergy(mcp->getEnergy());
				mcHadron->setMass(mcp->getMass());
				mcHadron->setCharge(mcp->getCharge());
				m_col_outputPfo->addElement( mcHadron );
			}
		}
		int n_Daughters = (mcp->getDaughters()).size();
		if ( n_Daughters != 0 )
		{
			for (int i_daughter = 0 ; i_daughter < n_Daughters ; ++i_daughter)
			{
				this->addMCPstoMCPartonShower( m_col_outputPfo , mcp->getDaughters()[i_daughter] );
			}
		}
}

void MCJetContent::end()
{

}
