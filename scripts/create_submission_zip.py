import os
import zipfile

def create_zip(output_zip_path):
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
    
    include_dirs = ['include', 'src', 'assets', 'tests', 'docs', 'scripts', 'saves']
    include_files = ['CMakeLists.txt', 'README.md', 'HOW_TO_RUN.md', 'Project - Go game.pdf']
    
    exclude_exts = {'.exe', '.obj', '.o', '.a', '.pdb', '.ilk', '.log', '.tmp'}
    exclude_dirs = {'build', 'build-core', '.git', '.vscode', '.gemini', 'scratch', '__pycache__'}
    
    with zipfile.ZipFile(output_zip_path, 'w', zipfile.ZIP_DEFLATED) as zf:
        for f in include_files:
            f_path = os.path.join(root_dir, f)
            if os.path.exists(f_path):
                arc_name = os.path.join('GoGame', f)
                zf.write(f_path, arc_name)
                print(f"Added file: {arc_name}")
                
        for d in include_dirs:
            dir_path = os.path.join(root_dir, d)
            if not os.path.exists(dir_path):
                continue
            
            # Ensure empty directories like saves/ are preserved
            if not os.listdir(dir_path):
                arc_name = os.path.join('GoGame', d) + '/'
                zfi = zipfile.ZipInfo(arc_name)
                zf.writestr(zfi, '')
                print(f"Added empty dir: {arc_name}")
                continue
                
            for root, dirs, files in os.walk(dir_path):
                dirs[:] = [sub for sub in dirs if sub not in exclude_dirs]
                for file in files:
                    ext = os.path.splitext(file)[1].lower()
                    if ext in exclude_exts or file.endswith('.gosave'):
                        continue
                    full_p = os.path.join(root, file)
                    rel_p = os.path.relpath(full_p, root_dir)
                    arc_name = os.path.join('GoGame', rel_p)
                    zf.write(full_p, arc_name)
                    print(f"Added: {arc_name}")

    print(f"\nCreated submission zip at: {output_zip_path}")
    print(f"File size: {os.path.getsize(output_zip_path)} bytes")

if __name__ == '__main__':
    zip_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'GoGame_Submission.zip'))
    create_zip(zip_path)
