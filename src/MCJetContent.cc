#include "MCJetContent.h"
#include <iostream>
#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>
#include <EVENT/ReconstructedParticle.h>
#include "IMPL/ReconstructedParticleImpl.h"
#include "IMPL/LCCollectionVec.h"
#include <EVENT/LCCollection.h>
#include "TLorentzVector.h"

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
m_minTheta(0.02),
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
					"MCJetContent_wInvisibles",
					"Name of output MCJetContent with Invisibles (neutrinos + undetectable particles escaping from beam pipe)",
					m_MCJetContent_wInvisibles,
					std::string("MCJetContent_wInvisibles")
				);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
					"MCJetContent_woInvisibles",
					"Name of output MCJetContent without Invisibles (neutrinos + undetectable particles escaping from beam pipe)",
					m_MCJetContent_woInvisibles,
					std::string("MCJetContent_woInvisibles")
				);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
					"MCISR",
					"Name of output MCISR collection",
					m_MCISR,
					std::string("MCISR")
				);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
					"MCFSR",
					"Name of output MCFSR collection",
					m_MCFSR,
					std::string("MCFSR")
				);

	registerOutputCollection(	LCIO::RECONSTRUCTEDPARTICLE,
					"MCIsolatedLeptons",
					"Name of output MCIsolatedLepton collection",
					m_MCIsolatedLeptons,
					std::string("MCIsolatedLeptons")
				);

	registerProcessorParameter(	"minTheta",
					"minimum theta for considering particles detectable in detector, default minTheta = 0.02 (20 mrad)",
					m_minTheta,
					float(0.02)
					);

	registerProcessorParameter(	"includeOverlay",
					"Whether include gamma-gamma -> Hadron overlay or not",
					m_includeOverlay,
					bool(false)
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
		outputPfoCollection = pLCEvent->getCollection(m_MCJetContent_wInvisibles);
		int n_inputHadrons = n_PartonShower;
		int n_outputPFOs = outputPfoCollection->getNumberOfElements();
		streamlog_out(DEBUG) << " CHECK : processed events: " << m_nEvtSum << " (Number of mcHadrons: " << n_inputHadrons << " , Number of outputHadrons: " << n_outputPFOs <<")" << std::endl;
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
		LCCollectionVec *m_col_outputPfo_wInvisibles = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		LCCollectionVec *m_col_outputPfo_woInvisibles = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		LCCollectionVec *m_col_outputPfo_ISR = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		LCCollectionVec *m_col_outputPfo_FSR = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		LCCollectionVec *m_col_outputPfo_IsoLep = new LCCollectionVec(LCIO::RECONSTRUCTEDPARTICLE);
		int n_MCP = MCParticleCollection->getNumberOfElements();
		std::vector<int> isolatedMuonIndex ; isolatedMuonIndex.clear();
		int ISOMuonIndex = -1;
		std::vector<int> isolatedAntiMuonIndex ; isolatedAntiMuonIndex.clear();
		int ISOMuonBarIndex = -1;
		std::vector<int> isolatedPhotonIndex ; isolatedPhotonIndex.clear();
		std::vector<int> FSRPhotonIndex ; FSRPhotonIndex.clear();
		for (int i_MCP = 0 ; i_MCP < n_MCP ; ++i_MCP)
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(i_MCP));
			if ( i_MCP == 8 || i_MCP == 9 )
			{
				this->finIsolatedLeptonFSR( pLCEvent , mcp , 13 , isolatedMuonIndex , isolatedPhotonIndex );
				this->finIsolatedLeptonFSR( pLCEvent , mcp , -13 , isolatedAntiMuonIndex , isolatedPhotonIndex );
			}
		}
		streamlog_out(DEBUG) << " " << std::endl;
		streamlog_out(DEBUG) << "In event "<< m_nEvt <<" , " << isolatedMuonIndex.size() << " Isolated Muon found" << std::endl;
		for ( unsigned int i_muon = 0 ; i_muon < isolatedMuonIndex.size() ; ++i_muon )
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(isolatedMuonIndex[i_muon]));
			streamlog_out(DEBUG) << "IsoLepton Index: " << isolatedMuonIndex[i_muon] << " , IsoLepton PDG = " << mcp->getPDG() << std::endl;
			ISOMuonIndex = isolatedMuonIndex[i_muon];
		}
		streamlog_out(DEBUG) << "In event "<< m_nEvt <<" , " << isolatedAntiMuonIndex.size() << " Isolated Anti-Muon found" << std::endl;
		for ( unsigned int i_muon = 0 ; i_muon < isolatedAntiMuonIndex.size() ; ++i_muon )
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(isolatedAntiMuonIndex[i_muon]));
			streamlog_out(DEBUG) << "IsoLepton Index: " << isolatedAntiMuonIndex[i_muon] << " , IsoLepton PDG = " << mcp->getPDG() << std::endl;
			ISOMuonBarIndex = isolatedAntiMuonIndex[i_muon];
		}
		for (int i_MCP = 0 ; i_MCP < n_MCP ; ++i_MCP)
		{
			int fsr = -1;
			for ( unsigned int i_fsr = 0 ; i_fsr < isolatedPhotonIndex.size() ; ++i_fsr )
			{
				if ( i_MCP == isolatedPhotonIndex[ i_fsr ] ) fsr = i_MCP;
			}
			if ( fsr != -1 ) FSRPhotonIndex.push_back( fsr );
		}
		streamlog_out(DEBUG) << "In event "<< m_nEvt <<" , " << isolatedPhotonIndex.size() << " ISOLated photon found" << std::endl;
		for ( unsigned int i_photon = 0 ; i_photon < isolatedPhotonIndex.size() ; ++i_photon )
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(isolatedPhotonIndex[i_photon]));
			streamlog_out(DEBUG) << "ISOLated photon Index: " << isolatedPhotonIndex[i_photon] << " , ISOLated photon Energy = " << mcp->getEnergy() << std::endl;
		}
		streamlog_out(DEBUG) << "In event "<< m_nEvt <<" , " << FSRPhotonIndex.size() << " FSR photon found" << std::endl;
		for ( unsigned int i_photon = 0 ; i_photon < FSRPhotonIndex.size() ; ++i_photon )
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(FSRPhotonIndex[i_photon]));
			streamlog_out(DEBUG) << "FSR photon Index: " << FSRPhotonIndex[i_photon] << " , FSR photon Energy = " << mcp->getEnergy() << std::endl;
		}
		streamlog_out(DEBUG) << "*********** event "<< m_nEvt <<" : Isolated Muon index = " << ISOMuonIndex << " ; Isolated AntiMuon index = " << ISOMuonBarIndex << " ***********" << std::endl;
		streamlog_out(DEBUG) << " " << std::endl;


		for (int i_MCP = 0 ; i_MCP < n_MCP ; ++i_MCP)
		{
			MCParticle* mcp = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(i_MCP));
			TVector3 mcHadronMomentum( mcp->getMomentum() );
			if ( mcp->getGeneratorStatus() != 1 ) continue;
			if ( !m_includeOverlay && mcp->isOverlay() ) continue;
			bool isFSR = false;
			for ( unsigned int i_photon = 0 ; i_photon < FSRPhotonIndex.size() ; ++i_photon )
			{
				if ( FSRPhotonIndex[ i_photon ] == i_MCP ) isFSR = true;
			}
			if ( abs( mcp->getPDG() ) == 13 && ( i_MCP == ISOMuonIndex || i_MCP == ISOMuonBarIndex ) )
			{
				ReconstructedParticleImpl* mcIsoLep = new ReconstructedParticleImpl;
				mcIsoLep->setType(mcp->getPDG());
				mcIsoLep->setMomentum(mcp->getMomentum());
				mcIsoLep->setEnergy(mcp->getEnergy());
				mcIsoLep->setMass(mcp->getMass());
				mcIsoLep->setCharge(mcp->getCharge());
				m_col_outputPfo_IsoLep->addElement( mcIsoLep );
				streamlog_out(MESSAGE) << "In event "<< m_nEvt <<" , An Isolated Muon added to " << m_MCIsolatedLeptons << " collection: IsoLepton PDG:" << mcp->getPDG() << " , IsoLepton Index:" << i_MCP << std::endl;
			}
			else
			{
				if ( ( i_MCP == 6 || i_MCP == 7 ) && mcp->getPDG() == 22 )
				{
					ReconstructedParticleImpl* mcISR = new ReconstructedParticleImpl;
					mcISR->setType(mcp->getPDG());
					mcISR->setMomentum(mcp->getMomentum());
					mcISR->setEnergy(mcp->getEnergy());
					mcISR->setMass(mcp->getMass());
					mcISR->setCharge(mcp->getCharge());
					m_col_outputPfo_ISR->addElement( mcISR );
					streamlog_out(DEBUG) << "An ISR photon added to " << m_MCISR << " collection" << std::endl;
				}
				else if ( isFSR )
				{
					ReconstructedParticleImpl* mcFSR = new ReconstructedParticleImpl;
					mcFSR->setType(mcp->getPDG());
					mcFSR->setMomentum(mcp->getMomentum());
					mcFSR->setEnergy(mcp->getEnergy());
					mcFSR->setMass(mcp->getMass());
					mcFSR->setCharge(mcp->getCharge());
					m_col_outputPfo_FSR->addElement( mcFSR );
					streamlog_out(DEBUG) << "A FSR photon added to " << m_MCFSR << " collection: FSR PDG:" << mcp->getPDG() << " , FSR Energy:" << mcp->getEnergy() << " GeV" << std::endl;
				}
				else if ( abs(mcp->getPDG()) == 12 || abs(mcp->getPDG()) == 14 || abs(mcp->getPDG()) == 16 || abs( cos( mcHadronMomentum.Theta() ) ) > cos( m_minTheta ) )
				{
					ReconstructedParticleImpl* mcHadron = new ReconstructedParticleImpl;
					mcHadron->setType(mcp->getPDG());
					mcHadron->setMomentum(mcp->getMomentum());
					mcHadron->setEnergy(mcp->getEnergy());
					mcHadron->setMass(mcp->getMass());
					mcHadron->setCharge(mcp->getCharge());
					m_col_outputPfo_wInvisibles->addElement( mcHadron );
					streamlog_out(DEBUG) << "An un-detectable mcParticle added to " << m_MCJetContent_wInvisibles << " collection" << std::endl;
				}
				else
				{
					ReconstructedParticleImpl* mcHadron_vis = new ReconstructedParticleImpl;
					mcHadron_vis->setType(mcp->getPDG());
					mcHadron_vis->setMomentum(mcp->getMomentum());
					mcHadron_vis->setEnergy(mcp->getEnergy());
					mcHadron_vis->setMass(mcp->getMass());
					mcHadron_vis->setCharge(mcp->getCharge());
					m_col_outputPfo_wInvisibles->addElement( mcHadron_vis );
					streamlog_out(DEBUG) << "A detectable mcParticle added to " << m_MCJetContent_wInvisibles << " collection" << std::endl;
					ReconstructedParticleImpl* mcHadron_invis = new ReconstructedParticleImpl;
					mcHadron_invis->setType(mcp->getPDG());
					mcHadron_invis->setMomentum(mcp->getMomentum());
					mcHadron_invis->setEnergy(mcp->getEnergy());
					mcHadron_invis->setMass(mcp->getMass());
					mcHadron_invis->setCharge(mcp->getCharge());
					m_col_outputPfo_woInvisibles->addElement( mcHadron_invis );
					streamlog_out(DEBUG) << "A detectable mcParticle added to " << m_MCJetContent_woInvisibles << " collection" << std::endl;
				}
			}
		}
		pLCEvent->addCollection( m_col_outputPfo_wInvisibles , m_MCJetContent_wInvisibles );
		pLCEvent->addCollection( m_col_outputPfo_woInvisibles , m_MCJetContent_woInvisibles );
		pLCEvent->addCollection( m_col_outputPfo_ISR , m_MCISR );
		pLCEvent->addCollection( m_col_outputPfo_FSR , m_MCFSR );
		pLCEvent->addCollection( m_col_outputPfo_IsoLep , m_MCIsolatedLeptons );
	}

        catch(DataNotAvailableException &e)
        {
		streamlog_out(MESSAGE) << "Input collection not found in event " << m_nEvt << std::endl;
        }

}

void MCJetContent::finIsolatedLeptonFSR( EVENT::LCEvent *pLCEvent , EVENT::MCParticle *mcp, int leptonPDG , std::vector<int> &isolatedLeptonIndex , std::vector<int> &isolatedPhotonIndex )
{
	LCCollection *MCParticleCollection = pLCEvent->getCollection(m_MCPcollection);
	int n_MCP = MCParticleCollection->getNumberOfElements();
	int mcpIndex = -1;
	int fsrIndex = -1;
	for (int i_MCP = 0 ; i_MCP < n_MCP ; ++i_MCP)
	{
		MCParticle* mcpTest = dynamic_cast<MCParticle*>(MCParticleCollection->getElementAt(i_MCP));
		if ( mcpTest == mcp && mcp->getPDG() == leptonPDG ) mcpIndex = i_MCP;
		if ( mcpTest == mcp && mcp->getPDG() == 22 ) fsrIndex = i_MCP;
	}
	if ( mcp->getGeneratorStatus() == 1 && mcp->getPDG() == leptonPDG )
	{
		isolatedLeptonIndex.push_back( mcpIndex );
	}
	else if ( mcp->getGeneratorStatus() == 1 && mcp->getPDG() == 22 )
	{
		isolatedPhotonIndex.push_back( fsrIndex );
	}
	int n_Daughters = (mcp->getDaughters()).size();
	if ( n_Daughters != 0 )
	{
		for (int i_daughter = 0 ; i_daughter < n_Daughters ; ++i_daughter)
		{
			this->finIsolatedLeptonFSR( pLCEvent , mcp->getDaughters()[i_daughter] , leptonPDG , isolatedLeptonIndex , isolatedPhotonIndex );
		}
	}
	


}

void MCJetContent::end()
{

}
