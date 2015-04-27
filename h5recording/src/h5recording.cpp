/* h5recording.cpp
 * Implementation of class representing a single MEA recording and
 * the HDF5 file to which it is saved
 */

#include <sys/stat.h>
#include <iostream>
#include <stdlib.h>

#include "h5recording.h"

using namespace H5;
using boost::extents;

H5Recording::H5Recording(std::string filename) {
	_filename = filename;

	/* If file exists, verify it is valid HDF5 and load data from it.
	 * Else, construct a new file.
	 */
	struct stat buffer;
	if (stat(_filename.c_str(), &buffer) == 0) {
		if (!H5File::isHdf5(_filename)) {
			std::cerr << "Invalid H5 file" << std::endl;
			exit(EXIT_FAILURE);
		}
		try {
			readOnly = true;
			file = H5File::H5File(_filename, H5F_ACC_RDONLY);
		} catch (FileIException &e) {
			std::cerr << "Could not open H5 file" << std::endl;
			exit(EXIT_FAILURE);
		}
		/* Get the data itself */
		dataset = file.openDataSet("data");
		dataspace = dataset.getSpace();

		/* Read attributes into data members */
		readIsLive();
		readLastValidSample();
		readFileType();
		readSampleRate();
		readBlockSize();
		readNumSamples();
		readNumChannels();
		readGain();
		readOffset();
		readDate();
		readTime();
		readRoom();

		/* Compute length of experiment */
		setLength(nsamples() / sampleRate());

	} else {
		/* New file. This needs to be only called by mealog... */
		readOnly = false;
		file = H5File(_filename, H5F_ACC_TRUNC);
		dataspace = DataSpace(H5Rec::DATASET_RANK, H5Rec::DATASET_DEFAULT_DIMS, 
				H5Rec::DATASET_MAX_DIMS);
		props = DSetCreatPropList();
		props.setChunk(H5Rec::DATASET_RANK, H5Rec::DATASET_CHUNK_DIMS);
		datatype = DataType(PredType::STD_I16LE);
		dataset = file.createDataSet("data", datatype, dataspace, props);

		/* Set default parameters */
		setFileType(H5Rec::BIN_FILE_TYPE);
		setFileVersion(H5Rec::BIN_FILE_VERSION);
		setNumChannels(H5Rec::NUM_CHANNELS);
		setBlockSize(H5Rec::BLOCK_SIZE);
		setSampleRate(H5Rec::SAMPLE_RATE);
		setRoom(H5Rec::DEFAULT_ROOM_STRING);
	}
}

H5Recording::~H5Recording() {
	try {
		if (!readOnly) 
			writeAllAttributes();
		this->file.close();
	} catch (FileIException &e) {
		std::cerr << "Error closing HDF5 file: " << this->_filename << std::endl;
	}
}

std::string H5Recording::filename(void) {
	return this->_filename;
}

double H5Recording::length(void) {
	return this->_length;
}

void H5Recording::setLength(double length) {
	this->_length = length;
	setNumSamples(length * H5Rec::SAMPLE_RATE);

	/* If this is a new recording, it will not be read-only. Requests
	 * to re-set the length of the file will be done by a class that
	 * is recording the data to disk.
	 */
	if (readOnly)
		return;
	hsize_t newSize[H5Rec::DATASET_RANK] = {H5Rec::NUM_CHANNELS, this->_nsamples};
	this->dataset.extend(newSize);
	this->dataspace = this->dataset.getSpace();
}

int16_t H5Recording::type(void) {
	return this->_type;
}

int16_t H5Recording::version(void) {
	return this->_version;
}

uint32_t H5Recording::nsamples(void) {
	return this->_nsamples;
}

uint32_t H5Recording::nchannels(void) {
	return this->_nchannels;
}

bool H5Recording::live(void) {
	return this->_live;
}

uint32_t H5Recording::lastValidSample(void) {
	return this->_lastValidSample;
}

uint32_t H5Recording::blockSize(void) {
	return this->_blockSize;
}

float H5Recording::sampleRate(void) {
	return this->_sampleRate;
}

float H5Recording::gain(void) {
	return this->_gain;
}

float H5Recording::offset(void) {
	return this->_offset;
}

std::string H5Recording::date(void) {
	return this->_date;
}

std::string H5Recording::time(void) {
	return this->_time;
}

std::string H5Recording::room(void) {
	return this->_room;
}

H5Rec::samples H5Recording::data(int startSample, int endSample) {
	/* Allocate return array */
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw;
	}
	H5Rec::samples s(extents[this->_nchannels][req_nsamples]);
	data(startSample, endSample, s);
	return s;
}

void H5Recording::data(int startSample, int endSample, H5Rec::samples &s) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw;
	}

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {this->_nchannels, 
			static_cast<hsize_t>(req_nsamples)};
	this->dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Boost multi-array, and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {this->_nchannels, 
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {this->_nchannels, 
		static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);

	/* Read data */
	this->dataset.read(s.data(), PredType::STD_I16LE, memspace, this->dataspace);
}

void H5Recording::data(int startSample, int endSample, H5Rec::samples_d &s) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw;
	}

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {this->_nchannels, 
			static_cast<hsize_t>(req_nsamples)};
	this->dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Boost multi-array, and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {this->_nchannels, 
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {this->_nchannels, 
		static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);

	/* Read data */
	this->dataset.read(s.data(), PredType::IEEE_F64LE, memspace, this->dataspace);

	/* Scale and offset by ADC properties */
	for (auto i = 0; i < this->_nchannels; i++) {
		for (auto j = 0; j < req_nsamples; j++)
			s[i][j] = (s[i][j] * this->_gain) + this->_offset;
	}
}

void H5Recording::setFilename(std::string filename) {
	this->_filename = filename;
}

void H5Recording::setData(int startSample, int endSample, 
		std::vector<std::vector<int16_t> > &data) {
}

void H5Recording::setData(int startSample, int endSample, H5Rec::samples &data) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples <= 0) {
		std::cerr << "Requested sample range is invalid: (" <<
				startSample << ", " << endSample << ")" << std::endl;
		throw;
	}

	/* Select hyperslab of dataspace where data will be written */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {this->_nchannels,
			static_cast<hsize_t>(req_nsamples)};
	this->dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);

	/* Define dataspace of memory region, from which data is read */
	hsize_t dims[H5Rec::DATASET_RANK] = {this->_nchannels, 
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {this->_nchannels, 
		static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);

	/* Write data 
	 * XXX: This is big-endian because it comes over the network. But 
	 * if we move away from sockets as the "source" of the data, this will
	 * likely change.
	 */
	this->dataset.write(data.origin(), PredType::STD_I16BE, 
			memspace, this->dataspace);
	this->flush();
}

void H5Recording::writeFileAttr(std::string name, const DataType &type, void *buf) {
	if (readOnly)
		return;
	try {
		DataType writeType(type);
		if (!(this->file.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			this->file.createAttribute(name, type, space);
		}
		Attribute attr = this->file.openAttribute(name);
		attr.write(writeType, buf);
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	} catch (FileIException &e) {
		std::cerr << "File exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeDataAttr(std::string name, const DataType &type, void *buf) {
	if (readOnly)
		return;
	try {
		DataType writeType(type);
		if (!(this->dataset.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			this->dataset.createAttribute(name, writeType, space);
		}
		Attribute attr = this->dataset.openAttribute(name);
		attr.write(writeType, buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeDataStringAttr(std::string name, std::string value) {
	if (readOnly)
		return;
	try {
		StrType stringType(0, value.length());
		if (!(this->dataset.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			this->dataset.createAttribute(name, stringType, space);
		}
		Attribute attr = this->dataset.openAttribute(name);
		attr.write(stringType, value.c_str());
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeAllAttributes(void) {
	writeFileAttr("is-live", PredType::STD_U8LE, &(this->_live));
	writeFileAttr("last-valid-sample", 
			PredType::STD_U64LE, &(this->_lastValidSample));
	writeDataAttr("bin-file-type", PredType::STD_I16LE, &(this->_type));
	writeDataAttr("bin-file-version", PredType::STD_I16LE, &(this->_version));
	writeDataAttr("sample-rate", PredType::STD_U32LE, &(this->_sampleRate));
	writeDataAttr("block-size", PredType::STD_U32LE, &(this->_blockSize));
	writeDataAttr("gain", PredType::IEEE_F32LE, &(this->_gain));
	writeDataAttr("offset", PredType::IEEE_F32LE, &(this->_offset));
	writeDataStringAttr("date", this->_date);
	writeDataStringAttr("time", this->_time);
	writeDataStringAttr("room", this->_room);
}

void H5Recording::setLive(bool live) {
	writeFileAttr("is-live", PredType::STD_U8LE, &live);
	this->_live = live;
}

void H5Recording::setLastValidSample(size_t sample) {
	writeFileAttr("last-valid-sample", PredType::STD_U64LE, &sample);
	this->_lastValidSample = sample;
}

void H5Recording::setFileType(int16_t type) {
	writeDataAttr("bin-file-type", PredType::STD_I16LE, &type);
	this->_type = type;
}

void H5Recording::setFileVersion(int16_t version) {
	writeDataAttr("bin-file-version", PredType::STD_I16LE, &version);
	this->_version = version;
}

void H5Recording::setSampleRate(float sampleRate) {
	writeDataAttr("sample-rate", PredType::IEEE_F32LE, &sampleRate);
	this->_sampleRate = sampleRate;
}

void H5Recording::setNumChannels(uint32_t nchannels) {
	this->_nchannels = nchannels;
}

void H5Recording::setNumSamples(uint32_t nsamples) {
	this->_nsamples = nsamples;
}

void H5Recording::setGain(float gain) {
	writeDataAttr("gain", PredType::IEEE_F32LE, &gain);
	this->_gain = gain;
}

void H5Recording::setOffset(float offset) {
	writeDataAttr("offset", PredType::IEEE_F32LE, &offset);
	this->_offset = offset;
}

void H5Recording::setBlockSize(size_t blockSize) {
	writeDataAttr("block-size", PredType::STD_U32LE, &blockSize);
	this->_blockSize = blockSize;
}

void H5Recording::setDate(std::string date) {
	writeDataStringAttr("date", date);
	this->_date = date;
}

void H5Recording::setTime(std::string time) {
	writeDataStringAttr("time", time);
	this->_time = time;
}

void H5Recording::setRoom(std::string room) {
	writeDataStringAttr("room", room);
	this->_room = room;
}

void H5Recording::readFileAttr(std::string name, void *buf) {
	try {
		Attribute attr = this->file.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (FileIException &e) {
		std::cerr << "File exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void H5Recording::readDataAttr(std::string name, void *buf) {
	try {
		Attribute attr = this->dataset.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void H5Recording::readDataStringAttr(std::string name, std::string &loc) {
	try {
		Attribute attr = this->dataset.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = (char *) calloc(sz + 1, 1);
		if (buf == NULL)
			throw;
		readDataAttr(name, buf);
		loc.replace(0, sz, buf);
		free(buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Calloc error" << std::endl;
	}
}

void H5Recording::readIsLive(void) {
	readFileAttr("is-live", &(this->_live));
}

void H5Recording::readLastValidSample(void) {
	readFileAttr("last-valid-sample", &(this->_lastValidSample));
}

void H5Recording::readFileType(void) {
	readDataAttr("bin-file-type", &(this->_type));
}

void H5Recording::readFileVersion(void) {
	readDataAttr("bin-file-version", &(this->_version));
}

void H5Recording::readSampleRate(void) {
	readDataAttr("sample-rate", &(this->_sampleRate));
}

void H5Recording::readBlockSize(void) {
	readDataAttr("block-size", &(this->_blockSize));
}

void H5Recording::readNumSamples(void) {
	hsize_t dims[2] = {0, 0};
	this->dataset.getSpace().getSimpleExtentDims(dims, NULL);
	this->_nsamples = dims[1];
}

void H5Recording::readNumChannels(void) {
	hsize_t dims[2] = {0, 0};
	this->dataset.getSpace().getSimpleExtentDims(dims, NULL);
	this->_nchannels = dims[0];
}

void H5Recording::readGain(void) {
	readDataAttr("gain", &(this->_gain));
}

void H5Recording::readOffset(void) {
	readDataAttr("offset", &(this->_offset));
}

void H5Recording::readDate(void) {
	readDataStringAttr("date", this->_date);
}

void H5Recording::readTime(void) {
	readDataStringAttr("time", this->_time);
}

void H5Recording::readRoom(void) {
	readDataStringAttr("room", this->_room);
}

void H5Recording::flush(void) {
	file.flush(H5F_SCOPE_GLOBAL);
}
