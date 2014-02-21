#include "PCAObject.h"

//-----------------------------------------------------------------------------
// 	scene::PCAObject
//-----------------------------------------------------------------------------

namespace scene {
	
void PCAObject::derivePCAModelFrom( const MeshObject& mo )
{	
	computePCA( const_cast<MeshObject&>(mo).meshBuffer(), meshBuffer(), m_pca, m_mshape );

	MeshObject::setMesh( &m_mshape, true );
}

void PCAObject::synthesize( const std::vector<double>& coefficients )
{
	// Create coefficient vector
	Eigen::VectorXd coeffs;
	coeffs = coeffs.Zero( m_pca.PC.cols() );

	int numCoeffs = std::min((int)coefficients.size(),(int)m_pca.PC.cols());
	for( int i=0; i < numCoeffs; i++ )
		coeffs(i) = coefficients[i];

	m_coeffs = coeffs;

	// Synthesize shape from PCA model
	Eigen::VectorXd synth =	m_pca.mu + m_pca.PC * (coeffs.cwiseProduct( m_pca.ev ));

	// Replace vertex buffer
	std::vector<float>& vbuf = meshBuffer().vbuffer();
	int n = std::min((int)vbuf.size(),(int)synth.size());
	
	#pragma omp parallel for
	for( int i=0; i < n; ++i )
		vbuf[i] = (float)synth(i);

	// Trigger update of GPU buffers
	meshBuffer().setFrameUpdateRequired();
}

void PCAObject::setFrame( int i )
{
	std::vector<double> coeffs( m_pca.PC.cols(), 0. );
	coeffs[i] = 2.0;
	synthesize( coeffs );
}

void PCAObject::getCoeffs( std::vector<double>& coeffs ) const
{
	coeffs.resize( m_coeffs.size() );
	for( int i=0; i < m_coeffs.size(); i++ )
		coeffs[i] = m_coeffs(i);
}

} // namespace scene 
