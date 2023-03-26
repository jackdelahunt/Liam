#include "compiler.h"

Module *create_lex_and_parse_module_from_path(
    u16 module_id, std::string module_name, std::filesystem::path module_path
) {
    auto files          = std::vector<File *>();
    u16 file_id_counter = 0;

    for (auto &direcotry_item : std::filesystem::directory_iterator(module_path))
    {
        if (!std::filesystem::is_regular_file(direcotry_item))
            continue;

        auto file_path = direcotry_item.path();

        if (!file_path.has_extension())
            continue;
        if (!(file_path.extension().string() == ".liam"))
            continue;

        Lexer lexer = Lexer(file_path);
        lexer.lex();

        Parser parser = Parser(file_path, &lexer.tokens);
        parser.parse();

        auto created_file     = parser.file;
        created_file->file_id = file_id_counter;
        files.push_back(created_file);
    }

    if (ErrorReporter::has_parse_errors())
    {
        for (auto &error : ErrorReporter::singleton->parse_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" + std::to_string(ErrorReporter::singleton->parse_errors.size()) +
            ")"
        );
    }

    return new Module(module_id, module_name, module_path, files);
}

std::vector<Module *> lex_parse(std::filesystem::path starting_import_path) {
    bool is_root = true;
    auto modules = std::vector<Module *>();

    // work queue for the loop below to work through
    // as more modules are found any module that has not been
    // created will be appended to this
    std::vector<std::filesystem::path> module_import_path_queue = {
        std::filesystem::path(starting_import_path.parent_path())};

    // a set containg all of the absolute paths of the modules that have been
    // created, this is used when checking an import to see if a module
    // needs to be created or skipped as it moght already exist
    auto already_imported_module_paths = std::unordered_map<std::string, int>();

    u16 module_id_counter = 0;

    while (module_import_path_queue.size() > 0)
    {
        auto current_module_path = module_import_path_queue.at(module_import_path_queue.size() - 1);
        module_import_path_queue.pop_back();

        std::string module_name;
        if (is_root)
        {
            module_name = "root";
            is_root     = false;
        }
        else
        { module_name = current_module_path.stem(); }

        auto created_module =
            create_lex_and_parse_module_from_path(module_id_counter, module_name, current_module_path);
        modules.push_back(created_module);
        module_id_counter++;

        // checking each import in every file and getting the absolute path
        // of that imported module, checking if it has been created already
        // and if not then it is added to the queue to be created
        for (auto &file : created_module->files)
        {
            for (auto import_stmt : file->top_level_import_statements)
            {
                auto import_path                          = import_stmt->path->token.string;
                auto module_path                          = created_module->path;
                auto module_path_and_relative_import_path = module_path.append(import_path);
                auto absolute_path = std::filesystem::absolute(module_path_and_relative_import_path).string();

                if (!(already_imported_module_paths.contains(absolute_path)))
                {
                    already_imported_module_paths[absolute_path] = 420;
                    module_import_path_queue.push_back(absolute_path);
                }
            }
        }
    }

    return modules;
}

void type_check(std::vector<Module *> *modules) {

    TypeChecker().type_check(modules);

    if (ErrorReporter::has_type_check_errors())
    {
        for (auto &error : ErrorReporter::singleton->type_check_errors)
        { error.print_error_message(); }

        panic(
            "Cannot continue with errors :: count (" +
            std::to_string(ErrorReporter::singleton->type_check_errors.size()) + ")"
        );
    }
}

std::string code_gen(std::vector<Module *> *modules) {
    return CppBackend().emit(modules);
}
