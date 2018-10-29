import os
import h5py
import struct

h5_path = '../../assets/ProbabilisticMorphableModels/data/'
# h5_path = "/media/chaiyujin/FE6C78966C784B81/Linux/ThirdParty/ProbabilisticMorphableModels/data/"

model_paths = {
    "head": "model2017-1_bfm_nomouth.h5",
    "face": "model2017-1_face12_nomouth.h5"
}


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
            fp.write(struct.pack('i', attr.ndim))
            for idim in range(attr.ndim):
                fp.write(struct.pack('i', attr.shape[idim]))
            attr.tofile(fp)
            # print("dim: {}, shape: {}".format(attr.ndim, attr.shape[:attr.ndim]))
        with open(path, "wb") as fp:
            bin_write(fp, data["mean"])
            bin_write(fp, data["pcaBasis"])
            bin_write(fp, data["pcaVariance"])
            bin_write(fp, data["noiseVariance"])

    def _export_color(self, output_dir):
        self.color = self._read_group("/color/")
        self._write(self.color, os.path.join(output_dir, "color"))

    def _export_shape(self, output_dir):
        self.shape = self._read_group("/shape/")
        self._write(self.shape, os.path.join(output_dir, "shape"))

    def _export_expression(self, output_dir):
        self.expression = self._read_group("/expression/")
        self._write(self.expression, os.path.join(output_dir, "expression"))

    def _export_landmark(self, output_dir):
        lms = self.file["/metadata/landmarks/text"].value.decode("utf-8").split("\n")
        while len(lms[-1]) == 0:
            lms = lms[:-1]
        with open(os.path.join(output_dir, "landmarks.txt"), "w") as fp:
            fp.write("\n".join(lms))
    
    def _export_representer(self, output_dir):
        def bin_write(fp, attr):
            fp.write(struct.pack('i', attr.ndim))
            for idim in range(attr.ndim):
                fp.write(struct.pack('i', attr.shape[idim]))
            attr.tofile(fp)
            print("dim: {}, shape: {}".format(attr.ndim, attr.shape[:attr.ndim]))
        with open(os.path.join(output_dir, "template.bin"), "wb") as fp:
            bin_write(fp, self.file["/shape/representer/cells"].value.T)
            bin_write(fp, self.file["/shape/representer/points"].value.T)

    def export(self, output_dir):
        os.makedirs(output_dir, exist_ok=True)
        self._export_color(output_dir)
        self._export_shape(output_dir)
        self._export_expression(output_dir)
        self._export_landmark(output_dir)
        self._export_representer(output_dir)


face_util = H5Util(os.path.join(h5_path, model_paths["face"]))
head_util = H5Util(os.path.join(h5_path, model_paths["head"]))
face_util.export("../assets/basel/{}/".format("face"))
head_util.export("../assets/basel/{}/".format("head"))
