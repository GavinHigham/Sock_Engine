BEGIN {
	print "//GENERATED FILE, CHANGES WILL BE LOST ON NEXT RUN OF MAKE."
	print "#ifndef MODELS_H"
	print "#define MODELS_H\n"
	print "#include <GL/glew.h>"
	print "#include \"buffer_group.h\"\n"
	color_scale = 1/255.0
}
FNR == 1 {
	split(FILENAME, arr, /[\/\.]/)
	name = arr[length(arr)-1]
	models[name]

	data[name, "positions"] = "\tGLfloat positions[] = {"
	data[name, "normals"] = "\tGLfloat normals[] = {"
	data[name, "colors"] = "\tGLfloat colors[] = {"
	data[name, "indices"] = "\tGLuint indices[] = {"
	reading_vertices = 0
	reading_indices = 0
	just_started = 1
}

($1 == "element" && $2 == "vertex") {vertex_count = $3 + 0}
($1 == "element" && $2 == "face") {face_count = $3 + 0}
($1 == "end_header") {reading_vertices = 1}
(vertex_count > 0 && reading_vertices) {
	if (!just_started) {
		vertex_count--
		if (vertex_count == 0) {
			reading_indices = 1
			reading_vertices = 0
			just_started = 1
		}
		data[name, "positions"] = data[name, "positions"] "\n\t\t" $1 ", " $2 ", " $3 ","
		data[name, "normals"] = data[name, "normals"] "\n\t\t" $4 ", " $5 ", " $6 ","
		data[name, "colors"] = data[name, "colors"] "\n\t\t" $7*color_scale ", " $8*color_scale ", " $9*color_scale ","
	} else {
		just_started = 0
	}
}
reading_indices {
	if (!just_started) {
		data[name, "indices"] = data[name, "indices"] "\n\t\t" $2 ", " $3 ", " $4 ","
		if ($1 == 4) {
			data[name, "indices"] = data[name, "indices"] " " $5 ","
		}
	} else {
		just_started = 0
	}
}
END {
	for (model_name in models) {
		print "int buffer_" model_name "(struct buffer_group bg)"
		print "{"
		print substr(data[model_name, "positions"], 1, length(data[model_name, "positions"])-1) "\n\t};\n"
		print substr(data[model_name, "normals"], 1, length(data[model_name, "normals"])-1) "\n\t};\n"
		print substr(data[model_name, "colors"], 1, length(data[model_name, "colors"])-1) "\n\t};\n"
		print substr(data[model_name, "indices"], 1, length(data[model_name, "indices"])-1) "\n\t};\n"
		print "\tglBindBuffer(GL_ARRAY_BUFFER, bg.vbo);"
		print "\tglBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);"
		print "\tglBindBuffer(GL_ARRAY_BUFFER, bg.cbo);"
		print "\tglBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);"
		print "\tglBindBuffer(GL_ARRAY_BUFFER, bg.nbo);"
		print "\tglBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);"
		print "\tglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bg.ibo);"
		print "\tglBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);"
		print "\treturn sizeof(indices)/sizeof(indices[0]);"
		print "}\n"
	}
	print "#endif"
}