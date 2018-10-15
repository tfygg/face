import os
import h5py

h5_path = '../assets/ProbabilisticMorphableModels/data/model2017-1_bfm_nomouth.h5'

class H5Util:
    def __init__(self, file_path):
        self.file = h5py.File(file_path,'r')
        self.color = None
        self.shape = None
        self.expression = None

    def _read_group(self, group):
        version = "{}.{}".format(
            self.file[os.path.join(group, "version/majorVersion")].value,
            self.file[os.path.join(group, "version/minorVersion")].value)
        model = self.file[os.path.join(group, "model")]
        return {
            "version": version,
            "mean": model["mean"].value,
            "pcaBasis": model["pcaBasis"].value,
            "pcaVariance": model["pcaVariance"].value,
            "noiseVariance": model["noiseVariance"].value
        }

    def _write(self, data, prefix):
        path = prefix + "_" + data["version"] + ".bin"
        def bin_write(fp, attr):
            pass
        print(data["mean"].shape, data["mean"].ndim)

    def _export_color(self, output_dir):
        self.color = self._read_group("/color/")
        self._write(self.color, os.path.join(output_dir, "color"))

    def _export_shape(self, output_dir):
        self.shape = self._read_group("/shape/")
        self._write(self.shape, os.path.join(output_dir, "shape"))

    def _export_expression(self, output_dir):
        self.expression = self._read_group("/expression/")
        self._write(self.expression, os.path.join(output_dir, "expression"))

    def export(self, output_dir):
        os.makedirs(output_dir, exist_ok=True)
        self._export_color(output_dir)
        self._export_shape(output_dir)
        self._export_expression(output_dir)


util = H5Util(h5_path)
util.export("../assets/basel/")
