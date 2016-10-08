/* hidensfile.h
 *
 * Implementation of simple subclass of the DataFile class, which represents
 * a file for data recorded from the HiDens array.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEAREC_HIDENSFILE_H_
#define MEAREC_HIDENSFILE_H_

#include "datafile.h"
#include "configuration.h"

/*! The hidensfile namespace contains portions of libdatafile that are
 * particular to the HiDens array data.
 */
namespace hidensfile {

/*! The default number of channels in a HiDens recording */
const int NUM_CHANNELS = 126;

/*! The HiDens data sample rate */
const float SAMPLE_RATE = 20000;

/*! The default array type */
const std::string DEFAULT_ARRAY("hidens");

/*! The HidensFile class is a Datafile subclass that provides extra functionality
 * specific to HiDens array recordings.
 */
class HidensFile : public datafile::DataFile {
	public:

		/*! Construct a HiDens recording file. */
		HidensFile(std::string filename, 
				std::string array = DEFAULT_ARRAY,
				int nchannels = NUM_CHANNELS);

		/*! Return the configuration saved in this file */
		Configuration configuration() const;

		/*! Return the list of x-positions for all connected electrodes */
		arma::Col<uint32_t> xpos() const;

		/*! Return the list of y-positions for all connected electrodes */
		arma::Col<uint32_t> ypos() const;

		/*! Return the list of x-indices for all connected electrodes */
		arma::Col<uint16_t> x() const;

		/*! Return the list of y-indices for all connected electrodes */
		arma::Col<uint16_t> y() const;

		/*! Return the list of labels for all connected electrodes */
		arma::Col<uint8_t> label() const;

		/*! Return the indices of all connected electrodes */
		arma::Col<uint32_t> indices() const;

		/*! Write the given configuration into the file */
		void setConfiguration(const Configuration&);

		/*! Override for the Hidens file class which enforces 
		 * that setting analog output is not supported for this
		 * class.
		 */
		virtual void setAnalogOutputSize(int sz) override;

	protected:
		void readConfiguration();
		void writeConfiguration();

		/* Read/write components of each Electrode struct.
		 * See include/hidensfile-templates.cc for implemenation
		 */
		template<class T>
		void readConfigurationDataset(const H5::DataSet& dset, T& out);
		template<class T>
		void writeConfigurationDataset(H5::DataSet& dset, T& out);

		Configuration configuration_;
		arma::Col<uint32_t> xpos_, ypos_;
		arma::Col<uint16_t> x_, y_;
		arma::Col<uint8_t> label_;
		arma::Col<uint32_t> indices_;

}; // end HidensFile class
}; // end hidensfile namespace

#include "hidensfile-templates.cc"

#endif

